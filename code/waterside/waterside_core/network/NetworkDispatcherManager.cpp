#include "NetworkDispatcherManager.h"

namespace waterside
{
	NetworkDispatcherManager::~NetworkDispatcherManager()
	{
		for (auto& item : mpDispatchers)
		{
			delete item.second;
		}
		for (auto& item : mpCoroDispatchers)
		{
			delete item.second;
		}
	}

	bool NetworkDispatcherManager::registerDispatcher(NetworkDispatcher* pDispatcher)
	{
		if (!mpDispatchers.emplace(pDispatcher->getId(), pDispatcher).second)
		{
			MLOG_ERROR(NET, "id={} has register", pDispatcher->getId());

			assert(false);
			delete pDispatcher;
			return false;
		}
		return true;
	}

	bool NetworkDispatcherManager::registerDispatcher(CoroNetworkDispatcher* pDispatcher)
	{
		if (!mpCoroDispatchers.emplace(pDispatcher->getId(), pDispatcher).second)
		{
			MLOG_ERROR(NET, "coro id={} has register", pDispatcher->getId());

			assert(false);
			delete pDispatcher;
			return false;
		}
		return true;
	}

	NetworkDispatcher* NetworkDispatcherManager::queryDispatcher(uint32_t id)
	{
		if (auto it = mpDispatchers.find(id); it != mpDispatchers.end())
			return it->second;
		return nullptr;
	}

	CoroNetworkDispatcher* NetworkDispatcherManager::queryCoroDispatcher(uint32_t id)
	{
		if (auto it = mpCoroDispatchers.find(id); it != mpCoroDispatchers.end())
			return it->second;
		return nullptr;
	}
}
