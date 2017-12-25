#pragma once

#include <v8.h>
#include <chrono>
#include <functional>

class Processor
{
public:
	using msg_func = std::function<void(v8::Isolate*, v8::Local<v8::Context>&)>;

    virtual ~Processor() = default;
    virtual void post(const msg_func& msg) = 0;

    static Processor* FromContext(const v8::Local<v8::Context>& ctx);
};

class V8ProcessorPool
{
public:
    using init_func = std::function<v8::Local<v8::Context>(v8::Isolate*)>;
    virtual ~V8ProcessorPool() = default;

    virtual void update_all() = 0;
    virtual std::shared_ptr<Processor> newProcessor(std::chrono::milliseconds processingTimePerStep, const init_func& init_ctx) = 0;

    static std::shared_ptr<V8ProcessorPool> Create(unsigned int parallelThreads);
};