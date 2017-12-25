#include <testx/testx.hpp>
#include <iostream>
#include "scripting/processor.hpp"

auto init_default_ctx(v8::Isolate* iso)
{
    return v8::Context::New(iso);
}

TESTX_AUTO_TEST_CASE(parallel_posting)
{
    const auto PROCESSOR_NUM = 64;
    auto pool = V8ProcessorPool::Create(4);

    std::atomic<int> sum{0};
    std::list<std::shared_ptr<Processor>> processors{};
    for(int i = 0; i < PROCESSOR_NUM; ++i) {
        processors.push_back(pool->newProcessor(std::chrono::milliseconds(500), &init_default_ctx));
    }

    for(const auto& p : processors) {
        p->post([&sum](v8::Isolate* iso, v8::Local<v8::Context>& ctx) {
            v8::Local<v8::String> source =
                v8::String::NewFromUtf8(iso, R"code(
                        for(var i = 0; i < 10; ++i)
                        {
                        };
                        5;
                    )code", v8::NewStringType::kNormal).ToLocalChecked();
            v8::Local<v8::Script> script = v8::Script::Compile(ctx, source).ToLocalChecked();
            auto integer = script->Run(ctx).ToLocalChecked()->ToUint32()->Value();
            sum += integer;
        });
    }

    pool->update_all();
    BOOST_CHECK_EQUAL(sum, PROCESSOR_NUM * 5);
}

using await_callback = std::function<void(v8::Isolate*, v8::Local<v8::Context>&, v8::Local<v8::Value>)>;

void resolve_await(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    auto iso = info.GetIsolate();
    v8::HandleScope hScope(iso);
    auto ctx = iso->GetCurrentContext();
    await_callback& callback = *((await_callback*)info.Data().As<v8::External>()->Value());
    callback(iso, ctx, info[0]);
}

void await(v8::Isolate* iso, v8::Local<v8::Context>& ctx, v8::Local<v8::Value> fResult, const await_callback& callback)
{
    if(fResult->IsPromise())
    {
        auto p = fResult.As<v8::Promise>();
        auto ctx = iso->GetCurrentContext();
        auto proc = reinterpret_cast<Processor*>(ctx->GetAlignedPointerFromEmbedderData(1));

        p->Then(ctx, v8::Function::New(ctx, resolve_await, v8::External::New(iso, new await_callback(callback)), 1).ToLocalChecked());
    }else{
        callback(iso, ctx, fResult);
    }
}

void call_next(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    auto iso = info.GetIsolate();
    v8::HandleScope hScope(iso);
    auto ctx = iso->GetCurrentContext();
    auto proc = Processor::FromContext(ctx);
    
    BOOST_REQUIRE(info[0]->IsFunction());
    //BOOST_CHECK(info[0]->IsAsyncFunction());

    auto func_l = v8::Local<v8::Function>::Cast(info[0]);
    v8::CopyablePersistentTraits<v8::Function>::CopyablePersistent func(iso, func_l);
    proc->post([func](v8::Isolate* iso, v8::Local<v8::Context>& ctx)
    {
        auto func_l = func.Get(iso);
        auto ret = func_l->Call(ctx->Global(), 0, nullptr);
        await(iso, ctx, ret, [](v8::Isolate*, v8::Local<v8::Context>&, v8::Local<v8::Value> res)
        {
            BOOST_CHECK_EQUAL(5, res->ToInt32()->Value());
        });
    });
}

void on_message(v8::Local<v8::Message> message, v8::Local<v8::Value> error)
{
    std::cout << "msg: " << *v8::String::Utf8Value(message->Get()) << std::endl;
    std::cout << "err: " << *v8::String::Utf8Value(error) << std::endl;
}

TESTX_AUTO_TEST_CASE(test_callback)
{
    const auto PROCESSOR_NUM = 128;
    auto pool = V8ProcessorPool::Create(4);

    auto proc = pool->newProcessor(std::chrono::milliseconds(500), [](auto iso) {
        auto global = v8::ObjectTemplate::New(iso);
        global->Set(v8::String::NewFromUtf8(iso, "do_next"), v8::FunctionTemplate::New(iso, &call_next));
        
        return v8::Context::New(iso, nullptr, global);
    });

    proc->post([&proc](v8::Isolate* iso, v8::Local<v8::Context>& ctx)
    {
        iso->AddMessageListener(&on_message);
        //ctx->Global()->Set(ctx, v8::String::NewFromUtf8(iso, "do_next"), v8::Function::New(iso, call_next));
        v8::Local<v8::String> source =
            v8::String::NewFromUtf8(iso, R"code(
                    do_next(function() {
                        return 5;
                    });
                    do_next(function() {
                        return Promise.resolve(5);
                    });
                    do_next(async function() {
                        return Promise.resolve(2 + 3);
                    });
                    do_next(async function() {
                        const x = await Promise.resolve(5);
                        return x;
                    })
                )code", v8::NewStringType::kNormal).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(ctx, source).ToLocalChecked();
        script->Run(ctx);
    });

    pool->update_all();
}
