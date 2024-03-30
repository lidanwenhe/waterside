#include "TcpSession.h"
#include "TcpNetwork.h"
#include "NetworkBase.h"
#include "Logger.h"

namespace waterside
{
	constexpr size_t TCP_MESSAGE_MAX_LENGTH = 8192;

	TcpSession::TcpSession(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor, boost::asio::ip::tcp::socket&& socket)
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
		mbHasClosed = true;
		boost::system::error_code ec;
		mTimer.cancel(ec);
		mSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		mSocket.close(ec);
	}

	void TcpSession::asyncCloseSocket()
	{
		if (mbHasClosed)
			return;

		mpExecutor->schedule([this, self = shared_from_this()] {
			closeSocket();
			});
	}

	async_simple::coro::Lazy<void> TcpSession::start()
	{
		onTimer(shared_from_this()).start([](async_simple::Try<void> Result) {
			if (Result.hasError())
			{
				std::exception_ptr error = Result.getException();
				try
				{
					std::rethrow_exception(error);
				}
				catch (const std::exception& e)
				{
					LOG_ERROR("Error Happened in task. {}", e.what());
				}
			}
			});

		char recvbuf[TCP_MESSAGE_MAX_LENGTH];
		size_t recvbytes = 0;

		for (;;)
		{
			auto [ec, length] = co_await async_read_some(mSocket, boost::asio::buffer(recvbuf + recvbytes, TCP_MESSAGE_MAX_LENGTH - recvbytes));
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

		MLOG_INFO(NET, "{} disconnected", getRemoteAddress());

		auto pContext = std::make_shared<NetworkContext>(mSessionId,
			RpcPacketHeader{
				MESSAGE_PACKET_TYPE_DISCONNECT,
				SERIALIZATION_TYPE_NO,
				0, 0, 0
			},
			std::vector<char>{});
		mpNetwork->addProcessPacket(pContext);
		

		closeSocket();

        mpNetwork->cleanSession(mSessionId);
	}

	async_simple::coro::Lazy<void> TcpSession::onTimer(std::shared_ptr<TcpSession> pSession)
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
						addProcessPacket();
						co_await onSending();

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
			co_await onTimer(pSession);
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

			auto [ec, _] = co_await async_write(mSocket, boost::asio::buffer(p->data(), p->size()));
			if (ec)
			{
				MLOG_WARN(NET, "async_write error:{}", ec.message());
				break;
			}
		}
	}

	std::string_view TcpSession::getRemoteAddress() const
	{
		if (mRemoteAddress.empty())
		{
			boost::system::error_code ec;
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

	std::string_view TcpSession::getLocalAddress() const
	{
		if (mLocalAddress.empty())
		{
			boost::system::error_code ec;
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
