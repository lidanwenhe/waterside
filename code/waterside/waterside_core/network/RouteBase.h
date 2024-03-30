#pragma once

#include "prerequisites.h"

namespace waterside
{
	struct RouteInfo
	{
		std::string serviceName;
		std::string ip;
		std::string port;
		SessionID sessionId;
		int32_t serverId;
	};

	class RouteBase
	{
	public:
		RouteBase() = default;

		~RouteBase() = default;

		bool addRouteInfo(std::string_view serviceName, std::string_view ip, std::string_view port, SessionID sessionId, int32_t serverId);

		void removeRouteInfo(SessionID sessionId);

		std::shared_ptr<RouteInfo> findRouteInfoByServiceName(std::string_view serviceName);

	private:
		std::unordered_map<SessionID, std::shared_ptr<RouteInfo>> mRouteInfoBySessionID;
		std::unordered_map<int32_t, std::shared_ptr<RouteInfo>> mRouteInfoByServerId;
		string_map<std::shared_ptr<RouteInfo>> mRouteInfoByServiceName;
	};
}