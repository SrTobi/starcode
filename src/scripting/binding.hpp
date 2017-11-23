#pragma once


#include <v8.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace bd {
    using namespace v8;

    template<typename T>
    inline Local<T> unwrap(const MaybeLocal<T>& maybe, const std::string& msg = "Failed to unwrap non existing local")
    {
        Local<T> ret;
        if(!maybe.ToLocal(&ret))
        {
            throw std::runtime_error(msg);
        }
        return ret;
    }

    inline Local<String> str(const char* s)
    {
        return unwrap(String::NewFromUtf8(Isolate::GetCurrent(), s, NewStringType::kNormal),
                        "Failed to create string");
    }

    inline Local<String> str(const std::string& s)
    {
        return unwrap(String::NewFromUtf8(Isolate::GetCurrent(), s.c_str(), NewStringType::kNormal, s.size()),
                        "Failed to create string");
    }


    namespace detail
    {
        template<typename T>
        struct converter {};

#define MAKE_NUMBER_CONVERTER(type, toFunc, fromFunc, v8Type, msg) \
            template<> \
            struct converter<type> \
            { \
                static inline type from_local(Local<Context> ctx, Local<Value> local) \
                { \
                    return unwrap(local->toFunc(ctx), msg)->Value(); \
                } \
                static inline Local<v8Type> to_local(Isolate* iso, type val) \
                { \
                    return fromFunc(iso, val); \
                } \
            };
        
        MAKE_NUMBER_CONVERTER(double, ToNumber, Number::New, Number, "Expected number");
        MAKE_NUMBER_CONVERTER(int64_t, ToInteger, Number::New, Number, "Expected integer");
        MAKE_NUMBER_CONVERTER(int32_t, ToInt32, Integer::New, Integer, "Expected integer");
        MAKE_NUMBER_CONVERTER(uint32_t, ToUint32, Integer::NewFromUnsigned, Integer, "Expected integer");
#undef MAKE_NUMBER_CONVERTER
        template<>
        struct converter<Local<Value>>
        {
            static inline Local<Value> from_local(Local<Context> ctx, Local<Value> local)
            {
                return local;
            }

            static inline Local<Value> to_local(Isolate* iso, Local<Value> local)
            {
                return local;
            }
        };

        template<>
        struct converter<std::string>
        {
            static inline std::string from_local(Local<Context> ctx, Local<Value> local)
            {
                String::Utf8Value utf8String(unwrap(local->ToString(ctx), "Failed to convert to string"));
                return std::string(*utf8String, utf8String.length());
            }

            static inline Local<String> to_local(Isolate* iso, const std::string& str)
            {
                return unwrap(String::NewFromUtf8(iso, str.c_str(), NewStringType::kNormal, str.size()), "Failed to convert string to local");
            }
        };
    }

    template<typename T>
    typename std::decay<T>::type fromLocal(Local<Context> ctx, Local<Value> local)
    {
        return detail::converter<typename std::decay<T>::type>::from_local(ctx, local);
    }


    template<typename T>
    Local<Value> toLocal(Isolate* iso, const T& val)
    {
        return detail::converter<typename std::decay<T>::type>::to_local(iso, val);
    }


    namespace detail {
        template<typename This, typename Ret, typename... Args>
        struct signature_wrapper
        {
            using this_type = This;

            constexpr static std::size_t arg_num = sizeof...(Args);

            template<void (This::*Func)(Args...), typename Dump = void>
            static auto callback_entry(const FunctionCallbackInfo<Value>& info) -> typename std::enable_if<std::is_void<Ret>::value, Dump>::type
            {
                void_callback_entry(Func, info, std::make_index_sequence<arg_num>());
            }

            template<void (This::*Func)(Args...) const, typename Dump = void>
            static auto callback_entry(const FunctionCallbackInfo<Value>& info) -> typename std::enable_if<std::is_void<Ret>::value, Dump>::type
            {
                void_callback_entry(Func, info, std::make_index_sequence<arg_num>());
            }

            template<Ret (This::*Func)(Args...), typename Dump = void>
            static auto callback_entry(const FunctionCallbackInfo<Value>& info) -> typename std::enable_if<!std::is_void<Ret>::value, Dump>::type
            {
                ret_callback_entry(Func, info, std::make_index_sequence<arg_num>());
            }

            template<Ret (This::*Func)(Args...) const, typename Dump = void>
            static auto callback_entry(const FunctionCallbackInfo<Value>& info) -> typename std::enable_if<!std::is_void<Ret>::value, Dump>::type
            {
                ret_callback_entry(Func, info, std::make_index_sequence<arg_num>());
            }

        private:
            template<typename Func, std::size_t... I>
            static void void_callback_entry(Func func, const FunctionCallbackInfo<Value>& info, std::index_sequence<I...>)
            {
                auto iso = info.GetIsolate();
                HandleScope hScope(iso);
                auto ctx = iso->GetCurrentContext();
                This* ths = reinterpret_cast<This*>(info.Data().As<External>()->Value());
                (ths->*func)(fromLocal<Args>(ctx, info[I])...);
            }

            template<typename Func, std::size_t... I>
            static void ret_callback_entry(Func func, const FunctionCallbackInfo<Value>& info, std::index_sequence<I...>)
            {
                auto iso = info.GetIsolate();
                HandleScope hScope(iso);
                auto ctx = iso->GetCurrentContext();
                This* ths = reinterpret_cast<This*>(info.Data().As<External>()->Value());
                info.GetReturnValue().Set(toLocal<Ret>(iso, (ths->*func)(fromLocal<Args>(ctx, info[I])...)));
            }
        };

        template<typename This, typename Ret, typename... Args>
        auto determine_signature(Ret (This::*)(Args...)) -> signature_wrapper<This, Ret, Args...>;

        template<typename This, typename Ret, typename... Args>
        auto determine_signature(Ret (This::*)(Args...) const) -> signature_wrapper<This, Ret, Args...>;

        template<typename FType, FType Func>
        struct function_wrapper
        {
            using signature = decltype(determine_signature(Func));

            template<typename T>
            static Local<FunctionTemplate> NewTemplate(Isolate* iso, T* ths)
            {
                return FunctionTemplate::New(iso, &signature::template callback_entry<Func>, External::New(iso, ths), Local<Signature>(), signature::arg_num);
            }

            template<typename T>
            static Local<Function> NewFunction(Local<Context> ctx, T* ths)
            {
                return unwrap(Function::New(ctx, &signature::template callback_entry<Func>, External::New(ctx->GetIsolate(), ths), Local<Signature>(), signature::arg_num),
                        "Failed to create new function");
            }
        };
    }

    #define FWrap(func)  ::bd::detail::function_wrapper<decltype(func), func>

    /*template<typename Func, typename This> //, typename Ret, typename... Args, Ret (This::*Func)(Args...)>
    Local<FunctionTemplate> new_function_template(Isolate* iso, This* _this)
    {
        using wrapper = decltype(detail::determine_callback_type(Func));
        return FunctionTemplate::New(iso, &wrapper::entry<Func>, External::New(iso, _this), Local<Signature>(), sizeof...(Args));
    }*/
}