#include "TcpNetwork.h"
#include "TcpSession.h"

namespace waterside
{
	TcpNetwork::TcpNetwork(std::size_t poolSize, asio::ip::tcp::endpoint&& endpoint)
        : mPool(poolSize)
        , mEndpoint(std::move(endpoint))
	{
        mThread = std::thread([&] { mPool.run(); });
	}

    async_simple::coro::Lazy<void> TcpNetwork::start()
	{
        auto& io_context = mPool.getIoContext();
        asio::ip::tcp::acceptor a(io_context, mEndpoint);
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            std::error_code ec;
            a.close(ec);
            });

        //std::cout << "Listen port " << 9980 << " successfully.\n";
        for (;;)
        {
            auto executor = mPool.getExecutor();
            asio::ip::tcp::socket socket(executor->getIoContext());

            auto error = co_await async_accept(a, socket);
            if (error)
            {
                //std::cout << "Accept failed, error: " << error.message()
                //    << '\n';
                if (error == asio::error::operation_aborted)
                    break;
                continue;
            }
            //std::cout << "New client coming.\n";
            auto pSession = std::make_shared<TcpSession>(this, getNewId(), executor, std::move(socket));
            addSession(pSession);
            pSession->start().via(executor).detach();
        }
	}

    async_simple::coro::Lazy<std::pair<std::error_code, SessionID>> TcpNetwork::connect(const std::string& host, const std::string& port)
    {
        auto executor = mPool.getExecutor();
        asio::ip::tcp::socket socket(executor->getIoContext());

        auto ec = co_await async_connect(executor->getIoContext(), socket, host, port);
        if (ec) {
            //std::cout << "Connect error: " << ec.message() << '\n';
            //throw asio::system_error(ec);
            co_return std::pair<std::error_code, SessionID>{ec, 0};
        }

        //std::cout << "Connect to " << host << ":" << port << " successfully.\n";

        auto pSession = std::make_shared<TcpSession>(this, getNewId(), executor, std::move(socket));
        addSession(pSession);
        pSession->start().via(executor).detach();

        co_return std::pair<std::error_code, SessionID>{std::error_code{}, pSession->getSessionId() };
    }

    void TcpNetwork::stop()
    {
        mPool.stop();
        if (mThread.joinable())
            mThread.join();
    }
}