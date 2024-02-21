#include "IoContextPool.h"

namespace waterside
{
    inline asio::io_context** getCurrentIoContext()
    {
        thread_local static asio::io_context* current = nullptr;
        return &current;
    }


    AsioExecutor::AsioExecutor(asio::io_context& ioContext)
        : mIoContext(ioContext)
    {
    }

    bool AsioExecutor::schedule(Func func)
    {
        asio::post(mIoContext, std::move(func));
        return true;
    }

    bool AsioExecutor::currentThreadInExecutor() const
    {
        auto ctx = getCurrentIoContext();
        return *ctx == &mIoContext;
    }

    size_t AsioExecutor::currentContextId() const
    {
        auto ctx = getCurrentIoContext();
        auto ptr = *ctx;
        return ptr ? (size_t)ptr : 0;
    };

    bool AsioExecutor::checkin(Func func, Context ctx)
    {
        auto& ioContext = *(asio::io_context*)ctx;
        asio::post(ioContext, std::move(func));
        return true;
    }


	IoContextPool::IoContextPool(std::size_t poolSize)
        : mNextIoContext(0)
    {
        if (poolSize == 0)
            poolSize = 1;

        for (std::size_t i = 0; i < poolSize; ++i)
        {
            auto io_context = std::make_shared<asio::io_context>();
            auto executor = std::make_unique<AsioExecutor>(*io_context);
            auto work = std::make_shared<asio::io_context::work>(*io_context);
            mIoContexts.emplace_back(io_context);
            mExecutors.emplace_back(std::move(executor));
            mWork.emplace_back(work);
        }
    }

    void IoContextPool::run()
    {
        std::vector<std::shared_ptr<std::thread>> threads;
        for (std::size_t i = 0; i < mIoContexts.size(); ++i)
        {
            threads.emplace_back(std::make_shared<std::thread>(
                [](io_context_ptr svr) {
                    auto ctx = getCurrentIoContext();
                    *ctx = svr.get();

                    svr->run(); 
                }, mIoContexts[i]));
        }

        for (std::size_t i = 0; i < threads.size(); ++i)
            threads[i]->join();
    }

    void IoContextPool::stop()
    {
        mWork.clear();

        for (std::size_t i = 0; i < mIoContexts.size(); ++i)
            mIoContexts[i]->stop();
    }

    asio::io_context& IoContextPool::getIoContext()
    {
        auto i = mNextIoContext.fetch_add(1, std::memory_order::relaxed);
        return *mIoContexts[i % mIoContexts.size()];
    }

    AsioExecutor* IoContextPool::getExecutor()
    {
        auto i = mNextIoContext.fetch_add(1, std::memory_order::relaxed);
        return mExecutors[i % mIoContexts.size()].get();
    }
}
