#include "RouteBase.h"
#include "Logger.h"

namespace waterside
{
	bool RouteBase::addRouteInfo(string_view serviceName, string_view ip, string_view port, SessionID sessionId, int32_t serverId)
	{
		if (auto it = mRouteInfoByServerId.find(serverId); it != mRouteInfoByServerId.end())
		{
			auto p = it->second;
			MLOG_ERROR(NET, "Route serverId={} already existed, serviceName={} addr={}:{}, exist serviceName={} addr={}:{}",
				serverId, serviceName, ip, port, p->serviceName, p->ip, p->port);
			return false;
		}
		if (auto it = mRouteInfoByServiceName.find(serviceName); it != mRouteInfoByServiceName.end())
		{
			auto p = it->second;
			MLOG_ERROR(NET, "Route serviceName={} already existed, serverId={} addr={}:{}, exist serverId={} addr={}:{}",
				serviceName, serverId, ip, port, p->serverId, p->ip, p->port);
			return false;
		}

		auto pInfo = std::make_shared<RouteInfo>(string{ serviceName }, string{ ip }, string{ port }, sessionId, serverId);
		
		mRouteInfoBySessionID[pInfo->sessionId] = pInfo;
		mRouteInfoByServerId[pInfo->serverId] = pInfo;
		mRouteInfoByServiceName[pInfo->serviceName] = pInfo;

		MLOG_INFO(NET, "Route + serviceName={} serverId={} addr={}:{}", serviceName, serverId, ip, port);
		return true;
	}

	void RouteBase::removeRouteInfo(SessionID sessionId)
	{
		if (auto it = mRouteInfoBySessionID.find(sessionId); it != mRouteInfoBySessionID.end())
		{
			auto p = it->second;
			MLOG_INFO(NET, "Route - serviceName={} serverId={} addr={}:{}", p->serviceName, p->serverId, p->ip, p->port);

			mRouteInfoByServerId.erase(p->serverId);
			mRouteInfoByServiceName.erase(p->serviceName);
			mRouteInfoBySessionID.erase(it);
		}
	}

	std::shared_ptr<RouteInfo> RouteBase::findRouteInfoByServiceName(string_view serviceName)
	{
		if (auto it = mRouteInfoByServiceName.find(serviceName); it != mRouteInfoByServiceName.end())
		{
			return it->second;
		}
		return nullptr;
	}
}
