#pragma once

#include "prerequisites.h"
#include "RouteBase.h"
#include "TcpNetwork.h"

namespace waterside
{
	bool onNameRegInfo(const NetworkContext& ctx, string_view serviceName, string_view ip, string_view port, int32_t serverId);
	std::tuple<string, string, int32_t> onFindNameRegInfoByServiceName(string_view serviceName);

	class Router : public RouteBase, public TLazySingleton<Router>
	{
	public:
		Router() = default;

		~Router() = default;

		void initialize(std::shared_ptr<TcpNetwork> pNetwork, string_view serviceName, string_view ip, string_view port,
			string_view nameRegIp, string_view nameRegPort, int32_t serverId)
		{
			mpNetwork = pNetwork;
			mServiceName = serviceName;
			mIp = ip;
			mPort = port;
			mNameRegIp = nameRegIp;
			mNameRegPort = nameRegPort;
			mServerId = serverId;
		}

		async_simple::coro::Lazy<bool> startNameReg();

		template <auto func, typename... Args>
		async_simple::coro::Lazy<std::optional<decltype(getReturnType<func>())>>
			call(string_view serviceName, Args... args)
		{
			auto pInfo = findRouteInfoByServiceName(serviceName);
			if (pInfo)
			{
				co_return co_await mpNetwork->call<func>(pInfo->sessionId, args...);
			}

			auto ok = co_await startNameReg();
			if (!ok)
			{
				co_return std::nullopt;
			}

			auto ret = co_await mpNetwork->call<onFindNameRegInfoByServiceName>(mNameRegSessionId, serviceName);
			if (ret)
			{
				auto [ip, port, serverId] = ret.value();
				if (ip.empty() || port.empty())
				{
					MLOG_WARN(NET, "onFindNameRegInfoByServiceName failed, serviceName={}", serviceName);

					co_return std::nullopt;
				}

				auto [ec, sessionId] = co_await mpNetwork->connect(ip, port);
				if (ec)
				{
					MLOG_WARN(NET, "Connect NameReg failed, error: {}", ec.message());
					co_return std::nullopt;
				}

				if (!addRouteInfo(serviceName, ip, port, sessionId, serverId))
				{
					co_return std::nullopt;
				}

				co_return co_await mpNetwork->call<func>(sessionId, args...);
			}
			else
			{
				MLOG_WARN(NET, "onFindNameRegInfoByServiceName ret failed, serviceName={}", serviceName);
				co_return std::nullopt;
			}
		}

	private:
		std::shared_ptr<TcpNetwork> mpNetwork;
		string mServiceName;
		string mIp;
		string mPort;
		string mNameRegIp;
		string mNameRegPort;
		SessionID mNameRegSessionId = 0;
		int32_t mServerId = 0;
	};
}
