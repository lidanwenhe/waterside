#include "Router.h"

namespace waterside
{
	async_simple::coro::Lazy<bool> Router::startNameReg()
	{
		if (mNameRegSessionId == 0)
		{
			auto [ec, sessionId] = co_await mpNetwork->connect(mNameRegIp, mNameRegPort);
			if (ec)
			{
				MLOG_WARN(NET, "Connect NameReg failed, error: {}", ec.message());
				co_return false;
			}

			auto ok = co_await mpNetwork->call<onNameRegInfo>(sessionId, mServiceName, mIp, mPort, mServerId);
			if (!ok || !ok.value())
			{
				MLOG_WARN(NET, "onNameRegInfo return failed, serviceName={} IP={} port={} serverId={}", mServiceName, mIp, mPort, mServerId);
				co_return false;
			}
			mNameRegSessionId = sessionId;
		}
		co_return true;
	}
}