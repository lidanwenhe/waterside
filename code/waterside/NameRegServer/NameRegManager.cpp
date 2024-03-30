#include "NameRegManager.h"

namespace waterside
{
	bool onNameRegInfo(const NetworkContext& ctx, std::string_view serviceName, std::string_view ip, std::string_view port, int32_t serverId)
	{
		return NameRegManager::instance().addRouteInfo(serviceName, ip, port, ctx.sessionId, serverId);
	}

	std::tuple<std::string, std::string, int32_t> onFindNameRegInfoByServiceName(std::string_view serviceName)
	{
		if (auto pInfo = NameRegManager::instance().findRouteInfoByServiceName(serviceName); pInfo)
		{
			return { pInfo->ip, pInfo->port, pInfo->serverId };
		}
		return { "", "", 0 };
	}

	void NameRegManager::onRegistHandler(TcpNetwork* pNetwork)
	{
		pNetwork->onDisconnectCallback = std::bind(&NameRegManager::removeRouteInfo, this, std::placeholders::_1);

		pNetwork->registHandler<onNameRegInfo>();
		pNetwork->registHandler<onFindNameRegInfoByServiceName>();
	}
}
