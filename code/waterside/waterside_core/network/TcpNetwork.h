#pragma once

#include "NetworkBase.h"

namespace waterside
{
	class TcpNetwork : public NetworkBase
	{
	public:
		TcpNetwork(std::size_t poolSize, asio::ip::tcp::endpoint&& endpoint);

		async_simple::coro::Lazy<void> start();

		async_simple::coro::Lazy<std::pair<std::error_code, SessionID>> connect(const std::string& host, const std::string& port);

		void stop();

	private:
		IoContextPool mPool;
		std::thread mThread;
		asio::ip::tcp::endpoint mEndpoint;
	};
}
