#pragma once

#include "SessionBase.h"

namespace waterside
{
	class TcpSession : public SessionBase, public std::enable_shared_from_this<TcpSession>
	{
	public:
		TcpSession(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor, boost::asio::ip::tcp::socket&& socket);

		virtual ~TcpSession();

		async_simple::coro::Lazy<void> start();

		virtual void asyncCloseSocket() override;

		virtual std::string_view getRemoteAddress() const override;
		virtual std::string_view getLocalAddress() const override;

	private:
		void closeSocket();

		async_simple::coro::Lazy<void> onTimer(std::shared_ptr<TcpSession> pSession);

		async_simple::coro::Lazy<void> onSending();

	private:
		boost::asio::ip::tcp::socket mSocket;
		boost::asio::steady_timer mTimer;
		bool mbHasClosed = false;
	};
}
