#pragma once

#include "prerequisites.h"

namespace waterside
{
	struct RouteInfo
	{
		string serviceName;
		string ip;
		string port;
		SessionID sessionId;
		int32_t serverId;
	};

	class RouteBase
	{
	public:
		RouteBase() = default;

		~RouteBase() = default;

		bool addRouteInfo(string_view serviceName, string_view ip, string_view port, SessionID sessionId, int32_t serverId);

		void removeRouteInfo(SessionID sessionId);

		std::shared_ptr<RouteInfo> findRouteInfoByServiceName(string_view serviceName);

	private:
		unordered_map<SessionID, std::shared_ptr<RouteInfo>> mRouteInfoBySessionID;
		unordered_map<int32_t, std::shared_ptr<RouteInfo>> mRouteInfoByServerId;
		string_map<std::shared_ptr<RouteInfo>> mRouteInfoByServiceName;
	};
}