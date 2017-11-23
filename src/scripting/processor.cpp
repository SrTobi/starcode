#include "processor.hpp"


#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <stack>
#include <thread>
#include <atomic>
#include <future>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>
#include <list>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <v8.h>
#include "libplatform/libplatform.h"


using namespace v8;


class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
 public:
	virtual void* Allocate(size_t length) {
	void* data = AllocateUninitialized(length);
	return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
	virtual void Free(void* data, size_t) { free(data); }
};

ArrayBufferAllocator allocator;





class V8Inst : public Processor
{
public:
	using init_func = V8ProcessorPool::init_func;

	V8Inst(boost::asio::io_service& service, const boost::posix_time::milliseconds& roundtime, const std::function<void()>& finished,
			std::promise<void> initPromise, const init_func& init_ctx)
		: mTimer(service)
		, mFinished(finished)
		, mInitPromise(std::move(initPromise))
        , mRoundTime(roundtime)
	{
		mIsolateThread = std::thread(std::bind(&V8Inst::run, this, init_ctx));
	}

	~V8Inst() override
	{
		mQuit = true;
		mIO.stop();
		if(mRun)
		{
			mIsolate->TerminateExecution();
		}
		mCV.notify_all();
		mIsolateThread.join();
	}

	virtual void post(const msg_func& msg) override
	{
		mIO.post(std::bind(&V8Inst::run_task, this, msg));
	}

	void start_round()
	{
		std::lock_guard<std::mutex> lock(mLockMutex);
		mRun = true;
		mTimer.expires_from_now(mRoundTime);
		mTimer.async_wait(std::bind(&V8Inst::run_interupt_requester, this));
		mCV.notify_all();
	}

private:
	void run(const init_func& init_ctx)
	{
		Isolate::CreateParams create_params;
		create_params.array_buffer_allocator = &allocator;
		mIsolate = Isolate::New(create_params);
		{
			Isolate::Scope isolate_scope(mIsolate);
			HandleScope handle_scope(mIsolate);
			mContext.Reset(mIsolate, init_ctx(mIsolate));

			mRun = true;
			mInitPromise.set_value();
			bool finished_a_round = false;
			while(!mQuit)
			{
				interupt(finished_a_round);
				mIO.poll();
				mIO.reset();
				finished_a_round = true;
			}
		}
	}

	void run_task(const msg_func& f)
	{
		HandleScope handleScope(mIsolate);
		auto ctx = Local<Context>::New(mIsolate, mContext);
		Context::Scope context_scope(ctx);
		f(mIsolate, ctx);
		mIsolate->RunMicrotasks();
	}

	void run_interupt_requester()
	{
		std::lock_guard<std::mutex> lock(mLockMutex);
		if(mRun)
			mIsolate->RequestInterrupt(&V8Inst::run_interupt, this);
	}

	static void run_interupt(Isolate* isolate, void* inst)
	{
		((V8Inst*)inst)->interupt(true);
	}

	void interupt(bool finished)
	{
		std::unique_lock<std::mutex> lock(mLockMutex);
		if(!mRun)
			return;
		mTimer.cancel();
		if(finished)
			mFinished();
		mRun = false;
		mCV.wait(lock, [this]{ return mRun || mQuit; });
	}


private:
	boost::asio::io_service mIO;
	std::thread mIsolateThread;
	Isolate* mIsolate;
	Persistent<Context> mContext;

    const boost::posix_time::milliseconds mRoundTime;
	boost::asio::deadline_timer mTimer;
	std::mutex mLockMutex;
	std::condition_variable mCV;
	std::atomic<bool> mRun{false};
	std::atomic<bool> mQuit{false};

	std::promise<void> mInitPromise;
	std::function<void()> mFinished;
};


class V8Manager : public V8ProcessorPool
{
public:
	using work = boost::asio::io_service::work;
	V8Manager(unsigned int numThreads)
		: mThreadCount(numThreads)
	{
	}
	
	~V8Manager()
	{
	}

	virtual std::shared_ptr<Processor> newProcessor(std::chrono::milliseconds processingTimePerStep, const init_func& init_ctx) override
	{
		std::promise<void> initP{};
		mInitFutures.emplace(initP.get_future());
        boost::posix_time::milliseconds procTime(processingTimePerStep.count());
		auto inst = std::make_shared<V8Inst>(mIO, procTime, std::bind(&V8Manager::do_task, this), std::move(initP), init_ctx);

		{
            mInsts.push_back(inst);
		}

		return inst;
	}

	virtual void update_all() override
	{
		while(mInitFutures.size())
		{
			mInitFutures.top().wait();
			mInitFutures.pop();
		}

		mIO.reset();
		//mWork = std::make_unique<work>(std::ref(mIO));
		mIt = mInsts.begin();

		for(unsigned int i = 0; i < mThreadCount; ++i)
		{
			post_task();
		}
		mIO.run();
		assert(mIt == mInsts.end());
	}

private:
	void post_task()
	{
		mIO.post(std::bind(&V8Manager::do_task, this));
	}

	void do_task()
	{
		std::shared_ptr<V8Inst> inst;
        while(!inst)
        {
			std::lock_guard<std::mutex> lock(mTaskMutex);

			if(mIt == mInsts.end())
			{
				return;
			}

            inst = *mIt;

            if(inst)
            {
                ++mIt;
            }else{
                mIt = mInsts.erase(mIt);
            }

		}

        inst->start_round();
	}

private:
	const unsigned int mThreadCount;
	std::list<std::shared_ptr<V8Inst>>::iterator mIt;
	std::mutex mTaskMutex;
	std::unique_ptr<work> mWork;
	std::list<std::shared_ptr<V8Inst>> mInsts;
	boost::asio::io_service mIO;
	std::stack<std::future<void>> mInitFutures;
};

namespace {
	struct V8Initializer
	{
		V8Initializer()
		{
			// Initialize V8.
			V8::InitializeICU();
			V8::InitializeExternalStartupData("~/workspace/starcode/build/bin/test-starcode");
			platform = std::unique_ptr<Platform>{platform::CreateDefaultPlatform()};
			V8::InitializePlatform(platform.get());
			V8::Initialize();
		}
		std::unique_ptr<Platform> platform;
	};
}

std::shared_ptr<V8ProcessorPool> V8ProcessorPool::Create(unsigned int parallelThreads)
{
	static V8Initializer initializer{};
	return std::make_shared<V8Manager>(parallelThreads);
}
