#pragma once

#include "NetworkBase.h"

namespace waterside
{
	class TcpNetwork : public NetworkBase, public std::enable_shared_from_this<TcpNetwork>
	{
	public:
		TcpNetwork(std::size_t poolSize, boost::asio::ip::tcp::endpoint&& endpoint);

		async_simple::coro::Lazy<void> start(std::function<void()> cb);

		void asyncClose();

		async_simple::coro::Lazy<std::pair<boost::system::error_code, SessionID>> connect(const std::string& host, const std::string& port);

		void release();

	protected:
		bool close();

	private:
		IoContextPool mPool;
		std::thread mThread;
		boost::asio::ip::tcp::endpoint mEndpoint;
		std::unique_ptr<boost::asio::ip::tcp::acceptor> mpAcceptor;
	};
}
