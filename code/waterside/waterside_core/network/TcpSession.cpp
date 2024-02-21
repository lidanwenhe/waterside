#include "TcpSession.h"
#include "TcpNetwork.h"
#include "NetworkBase.h"
#include "Logger.h"

namespace waterside
{
	constexpr size_t TCP_MESSAGE_MAX_LENGTH = 8192;

	TcpSession::TcpSession(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor, asio::ip::tcp::socket&& socket)
		: SessionBase(pNetwork, sessionId, executor)
        , mSocket(std::move(socket))
		, mTimer(executor->getIoContext())
	{
	}

	TcpSession::~TcpSession()
	{
		closeSocket();
	}

	void TcpSession::closeSocket()
	{
		if (mbHasClosed)
			return;
		std::error_code ec;
		mTimer.cancel(ec);
		mSocket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
		mSocket.close(ec);
		mbHasClosed = true;
	}

	async_simple::coro::Lazy<void> TcpSession::start()
	{
		onTimer().start([](async_simple::Try<void> Result) {
			if (Result.hasError())
				std::cout << "Error Happened in task.\n";
			else
				std::cout << "task completed successfully.\n";
			});

		char recvbuf[TCP_MESSAGE_MAX_LENGTH];
		size_t recvbytes = 0;

		for (;;)
		{
			auto [ec, length] = co_await async_read_some(mSocket, asio::buffer(recvbuf + recvbytes, TCP_MESSAGE_MAX_LENGTH - recvbytes));
			if (ec)
			{
				MLOG_WARN(NET, "async_read error:{}", ec.message());
				break;
			}

			recvbytes += length;

			if (!parseRecvPacket(recvbuf, recvbytes))
			{
				break;
			}
		}

        std::cout << "session thread id:" << std::this_thread::get_id()
            << std::endl;

		closeSocket();

        mpNetwork->cleanSession(mSessionId);
	}

	async_simple::coro::Lazy<void> TcpSession::onTimer()
	{
		if (mbHasClosed)
			co_return;

		mTimer.expires_from_now(std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<float>(0.1f)));
		/**mTimer.async_wait(
			[this, self = shared_from_this()](const asio::error_code& ec) {
				if (!ec)
				{
					//ELOGV(INFO, "close timeout client conn_id %d", conn_id_);

					if (!mbHasClosed)
					{
						//onSending();

						onTimer();
					}
					//closeSocket();
				}
			});*/

		auto ec = co_await async_timer_delay(mTimer);
		if (!ec) // check timer closed
		{
			addProcessPacket();
			co_await onSending();
			co_await onTimer();
		}
	}

	async_simple::coro::Lazy<void> TcpSession::onSending()
	{
		std::shared_ptr<SendBufInfo> p;
		for (uint32_t i = 0; i < mSendPacketLimit; i++)
		{
			if (mbHasClosed)
				break;

			if (!getSendPacket(p))
				break;

			auto [ec, _] = co_await async_write(mSocket, asio::buffer(p->data(), p->size()));
			if (ec)
			{
				MLOG_WARN(NET, "async_write error:{}", ec.message());
				break;
			}
		}
	}

	string TcpSession::getRemoteAddress() const
	{
		if (mRemoteAddress.empty())
		{
			std::error_code ec;
			auto ep = mSocket.remote_endpoint(ec);
			if (ec)
			{
				MLOG_ERROR(NET, "remote_endpoint error");
			}
			else
			{
				auto addr = ep.address();
				if (addr.is_v4())
				{
					mRemoteAddress = std::format("{}:{}", addr.to_v4().to_string(), ep.port());
				}
				else if (addr.is_v6())
				{
					mRemoteAddress = std::format("{}:{}", addr.to_v6().to_string(), ep.port());
				}
				else
				{
					MLOG_ERROR(NET, "remote_endpoint ip error");
				}
			}
		}
		return mRemoteAddress;
	}

	string TcpSession::getLocalAddress() const
	{
		if (mLocalAddress.empty())
		{
			std::error_code ec;
			auto ep = mSocket.local_endpoint(ec);
			if (ec)
			{
				MLOG_ERROR(NET, "local_endpoint error");
			}
			else
			{
				auto addr = ep.address();
				if (addr.is_v4())
				{
					mLocalAddress = std::format("{}:{}", addr.to_v4().to_string(), ep.port());
				}
				else if (addr.is_v6())
				{
					mLocalAddress = std::format("{}:{}", addr.to_v6().to_string(), ep.port());
				}
				else
				{
					MLOG_ERROR(NET, "local_endpoint ip error");
				}
			}
		}
		return mLocalAddress;
	}
}
