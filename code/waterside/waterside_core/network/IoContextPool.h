#pragma once

#include "prerequisites.h"
#include "async_simple/Executor.h"
#include "async_simple/coro/Lazy.h"
#include "ylt/util/type_traits.h"
#include "ylt/util/utils.hpp"

namespace waterside
{
    class AsioExecutor : public async_simple::Executor
    {
    public:
        AsioExecutor(boost::asio::io_context& ioContext);

        virtual bool schedule(Func func) override;

        virtual bool currentThreadInExecutor() const override;

        virtual size_t currentContextId() const override;

        virtual Context checkout() override { return &mIoContext; }
        virtual bool checkin(Func func, Context ctx) override;

        boost::asio::io_context& getIoContext() { return mIoContext; }

    private:
        boost::asio::io_context& mIoContext;
    };

	class IoContextPool
	{
	public:
		explicit IoContextPool(std::size_t poolSize);

        void run();

        void stop();

        volatile bool isStopped() const
        {
            return mbStopped;
        }
        std::size_t getPoolSize() const
        {
            return mPoolSize;
        }
        size_t currentIoContext()
        {
            return mNextIoContext - 1;
        }

        boost::asio::io_context& getIoContext();

        AsioExecutor* getExecutor();

    private:
        using io_context_ptr = std::shared_ptr<boost::asio::io_context>;
        using work_ptr = std::shared_ptr<boost::asio::io_context::work>;

        volatile bool mbStopped;
        std::size_t mPoolSize;

        std::vector<io_context_ptr> mIoContexts;
        std::vector<std::unique_ptr<AsioExecutor>> mExecutors;
        std::vector<work_ptr> mWork;
        std::atomic<std::size_t> mNextIoContext;
	};

    template <typename T>
        requires(!std::is_reference<T>::value)
    struct AsioCallbackAwaiter
    {
    public:
        using CallbackFunction = std::function<void(std::coroutine_handle<>, std::function<void(T)>)>;

        AsioCallbackAwaiter(CallbackFunction callbackFunction)
            : mCallbackFunction(std::move(callbackFunction)) {}

        bool await_ready() noexcept { return false; }

        void await_suspend(std::coroutine_handle<> handle)
        {
            mCallbackFunction(handle, [this](T t) { mResult = std::move(t); });
        }

        auto coAwait(async_simple::Executor* executor) noexcept
        {
            return std::move(*this);
        }

        T await_resume() noexcept { return std::move(mResult); }

    private:
        CallbackFunction mCallbackFunction;
        T mResult;
    };

    template <typename T>
    struct get_type_t
    {
        using type = T;
    };

    template <typename T>
    struct get_type_t<async_simple::coro::Lazy<T>>
    {
        using type = T;
    };

    template <auto func>
    constexpr auto getReturnType()
    {
        using Function = decltype(func);
        using return_type = typename get_type_t<coro_rpc::function_return_type_t<Function>>::type;
        if constexpr (std::is_void_v<return_type>)
        {
            return nullptr;
        }
        else
        {
            return return_type{};
        }
    }

    inline async_simple::coro::Lazy<boost::system::error_code> async_accept(
        boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket) noexcept {
        co_return co_await AsioCallbackAwaiter<boost::system::error_code>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) {
                acceptor.async_accept(
                    socket, [handle, set_resume_value = std::move(
                        set_resume_value)](auto ec) mutable {
                            set_resume_value(std::move(ec));
                            handle.resume();
                    });
                }};
    }

    template <typename Socket, typename AsioBuffer>
    inline async_simple::coro::Lazy<std::pair<boost::system::error_code, size_t>>
        async_read_some(Socket& socket, AsioBuffer&& buffer) noexcept {
        co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                socket.async_read_some(
                    std::move(buffer),
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec, auto size) mutable {
                            set_resume_value(std::make_pair(std::move(ec), size));
                            handle.resume();
                    });
                }};
    }

    template <typename Socket, typename AsioBuffer>
    inline async_simple::coro::Lazy<std::pair<boost::system::error_code, size_t>> async_read(
        Socket& socket, AsioBuffer& buffer) noexcept {
        co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                boost::asio::async_read(
                    socket, buffer,
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec, auto size) mutable {
                            set_resume_value(std::make_pair(std::move(ec), size));
                            handle.resume();
                    });
                }};
    }

    template <typename Socket, typename AsioBuffer>
    inline async_simple::coro::Lazy<std::pair<boost::system::error_code, size_t>>
        async_read_until(Socket& socket, AsioBuffer& buffer,
            boost::asio::string_view delim) noexcept {
        co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                boost::asio::async_read_until(
                    socket, buffer, delim,
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec, auto size) mutable {
                            set_resume_value(std::make_pair(std::move(ec), size));
                            handle.resume();
                    });
                }};
    }

    template <typename Socket, typename AsioBuffer>
    inline async_simple::coro::Lazy<std::pair<boost::system::error_code, size_t>> async_write(
        Socket& socket, AsioBuffer&& buffer) noexcept {
        co_return co_await AsioCallbackAwaiter<std::pair<boost::system::error_code, size_t>>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                boost::asio::async_write(
                    socket, std::move(buffer),
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec, auto size) mutable {
                            set_resume_value(std::make_pair(std::move(ec), size));
                            handle.resume();
                    });
                }};
    }

    inline async_simple::coro::Lazy<boost::system::error_code> async_connect(
        boost::asio::io_context& io_context, boost::asio::ip::tcp::socket& socket,
        const std::string& host, const std::string& port) noexcept {
        co_return co_await AsioCallbackAwaiter<boost::system::error_code>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                boost::asio::ip::tcp::resolver resolver(io_context);
                auto endpoints = resolver.resolve(host, port);
                boost::asio::async_connect(
                    socket, endpoints,
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec, auto size) mutable {
                            set_resume_value(std::move(ec));
                            handle.resume();
                    });
                }};
    }

    inline async_simple::coro::Lazy<boost::system::error_code> async_timer_delay(
        boost::asio::steady_timer& timer) noexcept {
        co_return co_await AsioCallbackAwaiter<boost::system::error_code>{
            [&](std::coroutine_handle<> handle, auto set_resume_value) mutable {
                timer.async_wait(
                    [handle, set_resume_value = std::move(set_resume_value)](
                        auto ec) mutable {
                            set_resume_value(std::move(ec));
                            handle.resume();
                    });
                }};
    }
}
