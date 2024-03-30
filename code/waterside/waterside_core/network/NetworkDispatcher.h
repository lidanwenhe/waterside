#pragma once

#include "prerequisites.h"
#include "SingleSignal.h"
#include "SessionBase.h"
#include "Logger.h"


namespace waterside
{
	// 消息分派器接口
	class NetworkDispatcher : private boost::noncopyable
	{
	public:
		NetworkDispatcher(uint32_t id)
			: mId(id)
		{
		}

		virtual ~NetworkDispatcher() = default;

		// 得到消息ID
		uint32_t getId() const
		{
			return mId;
		}

		// 处理消息
		virtual bool process(std::shared_ptr<NetworkContext> pContext) = 0;

	private:
		uint32_t mId;
	};


	template<typename T>
	class FbNetworkDispatcher final : public NetworkDispatcher
	{
	public:
		using Callback = bool(SessionID sessionId, const T*);
	
		FbNetworkDispatcher(uint32_t id, const std::function<Callback>& cb)
			: NetworkDispatcher(id)
		{
			mSignal.connect(cb);
		}

		virtual ~FbNetworkDispatcher() = default;

		// 处理消息
		virtual bool process(std::shared_ptr<NetworkContext> pContext) override
		{
			flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(pContext->body.data()), pContext->body.size());
			if (!verifier.VerifyBuffer<T>(nullptr))
			{
				MLOG_WARN(NET, "fb verify failed, id:{}", getId());
				return false;
			}

			return mSignal.emit(pContext->sessionId, flatbuffers::GetRoot<T>(pContext->body.data()));
		}

	private:
		TSingleSignal<Callback> mSignal;
	};

	class DefaultNetworkDispatcher final : public NetworkDispatcher
	{
	public:
		using Callback = bool(SessionID sessionId);

		DefaultNetworkDispatcher(uint32_t id, const std::function<Callback>& cb)
			: NetworkDispatcher(id)
		{
			mSignal.connect(cb);
		}

		virtual ~DefaultNetworkDispatcher() = default;

		// 处理消息
		virtual bool process(std::shared_ptr<NetworkContext> pContext) override
		{
			return mSignal.emit(pContext->sessionId);
		}

	private:
		TSingleSignal<Callback> mSignal;
	};
}

#define NEW_NETWORK_DISPATCHER(T, id, func) DEBUG_NEW waterside::FbNetworkDispatcher<T>(id, [this](SessionID sessionId, const T* fb) { return func(sessionId, fb); })
#define NEW_DEFAULT_NETWORK_DISPATCHER(id, func) DEBUG_NEW waterside::DefaultNetworkDispatcher(id, [this](SessionID sessionId) { return func(sessionId); })
