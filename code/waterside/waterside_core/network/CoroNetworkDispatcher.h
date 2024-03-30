#pragma once

#include "prerequisites.h"
#include "SessionBase.h"
#include "Logger.h"


namespace waterside
{
	// 消息分派器接口
	class CoroNetworkDispatcher : private boost::noncopyable
	{
	public:
		CoroNetworkDispatcher(uint32_t id)
			: mId(id)
		{
		}

		virtual ~CoroNetworkDispatcher() = default;

		// 得到消息ID
		uint32_t getId() const
		{
			return mId;
		}

		// 处理消息
		virtual async_simple::coro::Lazy<void> process(std::shared_ptr<NetworkContext> pContext) = 0;

	private:
		uint32_t mId;
	};


	template<typename T>
	class FbCoroNetworkDispatcher final : public CoroNetworkDispatcher
	{
	public:
		using Callback = std::function<async_simple::coro::Lazy<void>(SessionID sessionId, const T*)>;
	
		FbCoroNetworkDispatcher(uint32_t id, const Callback& cb)
			: CoroNetworkDispatcher(id)
			, mCallback(cb)
		{
		}

		virtual ~FbCoroNetworkDispatcher() = default;

		// 处理消息
		virtual async_simple::coro::Lazy<void> process(std::shared_ptr<NetworkContext> pContext) override
		{
			flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(pContext->body.data()), pContext->body.size());
			if (verifier.VerifyBuffer<T>(nullptr))
			{
				co_await mCallback(pContext->sessionId, flatbuffers::GetRoot<T>(pContext->body.data()));
			}
			else
			{
				MLOG_WARN(NET, "fb verify failed, id:{}", getId());
			}
		}

	private:
		Callback mCallback;
	};

	class DefaultCoroNetworkDispatcher final : public CoroNetworkDispatcher
	{
	public:
		using Callback = std::function<async_simple::coro::Lazy<void>(SessionID sessionId)>;

		DefaultCoroNetworkDispatcher(uint32_t id, const Callback& cb)
			: CoroNetworkDispatcher(id)
			, mCallback(cb)
		{
		}

		virtual ~DefaultCoroNetworkDispatcher() = default;

		// 处理消息
		virtual async_simple::coro::Lazy<void> process(std::shared_ptr<NetworkContext> pContext) override
		{
			co_await mCallback(pContext->sessionId);
		}

	private:
		Callback mCallback;
	};
}

#define NEW_CORO_NETWORK_DISPATCHER(T, id, func) DEBUG_NEW waterside::FbCoroNetworkDispatcher<T>(id, [this](SessionID sessionId, const T* fb) -> async_simple::coro::Lazy<void> { co_await func(sessionId, fb); })
#define NEW_DEFAULT_CORO_NETWORK_DISPATCHER(id, func) DEBUG_NEW waterside::DefaultCoroNetworkDispatcher(id, [this](SessionID sessionId) -> async_simple::coro::Lazy<void> { co_await func(sessionId); })
