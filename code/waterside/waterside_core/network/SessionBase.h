#pragma once

#include "IoContextPool.h"
#include "rpc_protocol_generated.h"

namespace waterside
{
	class NetworkBase;

	struct NetworkContext
	{
		SessionID sessionId = 0;
		RpcPacketHeader header;
		vector<char> body;
	};

	constexpr size_t SESSION_SEND_MESSAGE_LENGTH = 1400;

	class SessionBase
	{
	public:
		SessionBase(NetworkBase* pNetwork, SessionID sessionId, AsioExecutor* executor);

		virtual ~SessionBase() = default;

		virtual void asyncCloseSocket() {}

		void send(const RpcPacketHeader& header, const std::pair<const void*, size_t>& body);

		SessionID getSessionId() const
		{
			return mSessionId;
		}

		virtual string_view getRemoteAddress() const = 0;
		virtual string_view getLocalAddress() const = 0;

	protected:
		bool parseRecvPacket(char* buf, size_t& recvbytes);

		void addProcessPacket();

		class SendBufInfo
		{
			char buf[SESSION_SEND_MESSAGE_LENGTH];
			size_t used = 0;
		public:
			char* data() { return buf; }
			size_t size() { return used; }
			size_t addData(const void* data, size_t len);
		};
		bool getSendPacket(std::shared_ptr<SendBufInfo>& p);

	protected:
		NetworkBase* mpNetwork;
		SessionID mSessionId;
		AsioExecutor* mpExecutor;
		mutable string mRemoteAddress;
		mutable string mLocalAddress;
		
		uint32_t mSendPacketLimit = 100;
		uint32_t mRecvPacketLimit = 100;
		uint32_t mProcessPacketLimit = 100;

	private:
		deque<std::shared_ptr<SendBufInfo>> mSendBufs;
		std::mutex mSendMutex;

		deque<std::shared_ptr<NetworkContext>> mRecvPackets;
	};
}
