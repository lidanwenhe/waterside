#pragma once

#include "SessionBase.h"

namespace waterside
{
	class TcpSession : public SessionBase
	{
	public:
		TcpSession(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor, asio::ip::tcp::socket&& socket);

		virtual ~TcpSession();

		async_simple::coro::Lazy<void> start();

		virtual string getRemoteAddress() const override;
		virtual string getLocalAddress() const override;

	private:
		void closeSocket();

		async_simple::coro::Lazy<void> onTimer();

		async_simple::coro::Lazy<void> onSending();

	private:
		asio::ip::tcp::socket mSocket;
		asio::steady_timer mTimer;
		bool mbHasClosed = false;
	};
}
