#include <testx/testx.hpp>
#include <iostream>
#include "scripting/processor.hpp"
#include "scripting/binding.hpp"

using namespace bd;
using namespace v8;

enum class MockEvent
{
    Called,
    Ending
};

template<typename C>
std::basic_ostream<C>& operator <<(std::basic_ostream<C>& s, MockEvent e)
{
    s << "[MockEvent]";
    return s;
}


template<typename Fixture, typename TestT>
struct FixtureWrapper
{
    FixtureWrapper(TestT eres, std::string code)
        : expected_res(eres)
        , code(code)
    {
        mockObserver.set()
            << MockEvent::Ending;
    }

    TestT expected_res;
    testx::MockObserver<MockEvent> mockObserver;
    Fixture fixture;
    std::string code;
    int assertCalls = 0;

    void assertImpl(bool test, const std::string& msg)
    {
        std::cout << "assert" << std::endl;
        BOOST_CHECK_MESSAGE(test, msg);
        ++assertCalls;
    }

    void check()
    {
        auto pool = V8ProcessorPool::Create(1);

        auto proc = pool->newProcessor(std::chrono::milliseconds(1000), [this](auto iso) {
            auto global = ObjectTemplate::New(iso);
            global->Set(str("test"), FWrap(&Fixture::test)::NewTemplate(iso, &fixture));
            global->Set(str("assert"), FWrap(&FixtureWrapper::assertImpl)::NewTemplate(iso, &fixture));
            
            return Context::New(iso, nullptr, global);
        });

        proc->post([&proc, this](Isolate* iso, Local<Context>& ctx)
        {
            Local<String> source = str(code);
            Local<Script> script = Script::Compile(ctx, source).ToLocalChecked();
            auto res = fromLocal<TestT>(iso, ctx, unwrap(script->Run(ctx)));
            BOOST_CHECK_EQUAL(res, expected_res);
        });

        pool->update_all();
        pool->update_all();
        mockObserver.expect(MockEvent::Ending);

        int needed_asserts = fixture.assert_calls;
        BOOST_CHECK_EQUAL(needed_asserts, assertCalls);
    }
};

#define TEST_BINDING(testT, fixture, code, result) \
    TESTX_AUTO_TEST_CASE(fixture##_test) \
    { \
        FixtureWrapper<fixture, testT> f(result, code); \
        f.check(); \
    }


struct test_int_binding
{
    const int assert_calls = 0;
    int test(int i1, const int& i2)
    {
        BOOST_CHECK_EQUAL(i1, 111);
        BOOST_CHECK_EQUAL(i2, 222);

        return 999;
    }
};
TEST_BINDING(int, test_int_binding, R"(test(111, "222"))", 999);


struct test_string_binding
{
    const int assert_calls = 0;
    std::string test(std::string i1, const std::string& i2)
    {
        BOOST_CHECK_EQUAL(i1, "one");
        BOOST_CHECK_EQUAL(i2, "two");

        return "return";
    }
};
TEST_BINDING(std::string, test_string_binding, R"(test("one", "two"))", "return");

struct test_double_binding
{
    const int assert_calls = 0;
    double test(double i1, const double& i2)
    {
        BOOST_CHECK_CLOSE(i1, 100.0, 0.1);
        BOOST_CHECK_CLOSE(i2, 66.6, 0.1);

        return 1.0;
    }
};
TEST_BINDING(double, test_double_binding, R"(test(100.0, 66.6))", 1.0);

struct test_function_binding
{
    const int assert_calls = 1;
    std::string test(const std::function<void(int i)>& f)
    {
        std::cout << "before call" << std::endl;
        f(5);
        std::cout << "after call" << std::endl;

        return "blub";
    }
};
TEST_BINDING(std::string, test_function_binding, R"(test((i) => {assert(i == 5, "i should be 5")}))", "blub");
