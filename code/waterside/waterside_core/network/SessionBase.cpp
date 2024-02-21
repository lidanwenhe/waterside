#include "SessionBase.h"
#include "TcpNetwork.h"
#include "Logger.h"

namespace waterside
{
	constexpr size_t TCP_MESSAGE_MAX_LENGTH = 8192;

	SessionBase::SessionBase(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor)
		: mpNetwork(pNetwork)
        , mSessionId(sessionId)
        , mpExecutor(executor)
	{
	}

	bool SessionBase::parseRecvPacket(char* buf, size_t& recvbytes)
	{
		size_t offset = 0;
		while (recvbytes - offset >= sizeof(RpcPacketHeader))
		{
			auto header = reinterpret_cast<RpcPacketHeader*>(buf + offset);
			size_t needbytes = header->length() + sizeof(RpcPacketHeader);
			if (needbytes > SESSION_SEND_MESSAGE_LENGTH)
			{
				MLOG_WARN(NET, "prase packet length error:{}, function_id:{}", header->length(), header->function_id());
				return false;
			}

			if (recvbytes - offset >= needbytes)
			{
				if (mRecvPackets.size() >= mRecvPacketLimit)
				{
					MLOG_WARN(NET, "recv packet out of limit:{}, function_id:{}", mRecvPacketLimit, header->function_id());
					return false;
				}

				auto p = reinterpret_cast<char*>(header + 1);
				mRecvPackets.emplace_back(std::make_shared<NetworkContext>(mSessionId, *header, vector<char>(p, p + header->length())));

				offset += needbytes;
			}
		}

		if (offset >= recvbytes)
		{
			recvbytes = 0;
		}
		else
		{
			memmove(buf, buf + offset, recvbytes - offset);
			recvbytes -= offset;
		}
		return true;
	}

	void SessionBase::addProcessPacket()
	{
		for (uint32_t i = 0; i < mProcessPacketLimit; i++)
		{
			if (mRecvPackets.empty())
				break;

			mpNetwork->addProcessPacket(mRecvPackets.front());
			mRecvPackets.pop_front();
		}
	}

	bool SessionBase::getSendPacket(std::shared_ptr<SendBufInfo>& p)
	{
		std::lock_guard lock(mSendMutex);
		if (mSendBufs.empty())
			return false;

		p = mSendBufs.front();
		mSendBufs.pop_front();
		return true;
	}

	size_t SessionBase::SendBufInfo::addData(const void* data, size_t len)
	{
		if (data == nullptr || len == 0)
			return 0;

		if (used == SESSION_SEND_MESSAGE_LENGTH)
			return 0;

		size_t sz = std::min(SESSION_SEND_MESSAGE_LENGTH - used, len);
		memcpy(buf + used, data, sz);
		used += sz;
		return sz;
	}

	void SessionBase::send(const RpcPacketHeader& header, const std::pair<const void*, size_t>& body)
	{
		std::lock_guard lock(mSendMutex);
		std::shared_ptr<SendBufInfo> p;
		if (!mSendBufs.empty() && mSendBufs.back()->size() < SESSION_SEND_MESSAGE_LENGTH)
		{
			p = mSendBufs.back();
		}
		else
		{
			p = std::make_shared<SendBufInfo>();
			mSendBufs.push_back(p);
		}

		size_t sz = p->addData(&header, sizeof(RpcPacketHeader));
		if (sz < sizeof(RpcPacketHeader))
		{
			p = std::make_shared<SendBufInfo>();
			mSendBufs.push_back(p);

			p->addData(reinterpret_cast<const char*>(&header) + sz, sizeof(RpcPacketHeader) - sz);
		}

		const char* data = reinterpret_cast<const char*>(body.first);
		size_t len = body.second;
		for (;;)
		{
			sz = p->addData(data, len);
			if (sz >= len)
				break;
			data += sz;
			len -= sz;

			p = std::make_shared<SendBufInfo>();
			mSendBufs.push_back(p);
		}
	}
}
