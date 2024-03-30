#pragma once

#include "NetworkDispatcher.h"
#include "CoroNetworkDispatcher.h"

namespace waterside
{
	class NetworkDispatcherManager : private boost::noncopyable
	{
	public:
		NetworkDispatcherManager() = default;

		~NetworkDispatcherManager();

		// 注册消息分派器
		bool registerDispatcher(NetworkDispatcher* pDispatcher);
		bool registerDispatcher(CoroNetworkDispatcher* pDispatcher);

		// 查询消息分派器
		NetworkDispatcher* queryDispatcher(uint32_t id);
		CoroNetworkDispatcher* queryCoroDispatcher(uint32_t id);

	private:
		std::unordered_map<uint32_t, NetworkDispatcher*> mpDispatchers;
		std::unordered_map<uint32_t, CoroNetworkDispatcher*> mpCoroDispatchers;
	};
}
