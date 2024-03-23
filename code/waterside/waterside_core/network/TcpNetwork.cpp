#include "TcpNetwork.h"
#include "TcpSession.h"

namespace waterside
{
	TcpNetwork::TcpNetwork(std::size_t poolSize, boost::asio::ip::tcp::endpoint&& endpoint)
        : mPool(poolSize)
        , mEndpoint(std::move(endpoint))
	{
        mThread = std::thread([&] { mPool.run(); });
	}

    async_simple::coro::Lazy<void> TcpNetwork::start(std::function<void()> cb)
	{
        auto& io_context = mPool.getIoContext();
        mpAcceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(io_context, mEndpoint);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&, this](auto, auto) {
            if (close())
                cb();
            });

        MLOG_INFO(NET, "Listen port {} successfully", mEndpoint.port());

        for (;;)
        {
            auto executor = mPool.getExecutor();
            boost::asio::ip::tcp::socket socket(executor->getIoContext());

            auto ec = co_await async_accept(*mpAcceptor, socket);
            if (ec)
            {
                MLOG_WARN(NET, "Accept failed, error: {}", ec.message());
                if (ec == boost::asio::error::operation_aborted)
                    break;
                continue;
            }

            auto pSession = std::make_shared<TcpSession>(this, getNewId(), executor, std::move(socket));
            addSession(pSession);
            pSession->start().via(executor).detach();

            MLOG_INFO(NET, "Accept from {} successfully", pSession->getRemoteAddress());
        }
        
        closeAllSession();
	}

    bool TcpNetwork::close()
    {
        if (mpAcceptor)
        {
            boost::system::error_code ec;
            mpAcceptor->close(ec);
            mpAcceptor.reset();
            return true;
        }
        return false;
    }

    void TcpNetwork::asyncClose()
    {
        mPool.getExecutor()->schedule([this, self = shared_from_this()] {
            this->close();
            });
    }

    async_simple::coro::Lazy<std::pair<boost::system::error_code, SessionID>> TcpNetwork::connect(const std::string& host, const std::string& port)
    {
        auto executor = mPool.getExecutor();
        boost::asio::ip::tcp::socket socket(executor->getIoContext());

        auto ec = co_await async_connect(executor->getIoContext(), socket, host, port);
        if (ec)
        {
            MLOG_ERROR(NET, "Connect error: {}", ec.message());
            //throw asio::system_error(ec);
            co_return std::pair<boost::system::error_code, SessionID>{ec, 0};
        }

        MLOG_INFO(NET, "Connect to {}:{} successfully", host, port);

        auto pSession = std::make_shared<TcpSession>(this, getNewId(), executor, std::move(socket));
        addSession(pSession);
        pSession->start().via(executor).detach();

        co_return std::pair<boost::system::error_code, SessionID>{boost::system::error_code{}, pSession->getSessionId() };
    }

    void TcpNetwork::release()
    {
        mPool.stop();
        if (mThread.joinable())
            mThread.join();
    }
}