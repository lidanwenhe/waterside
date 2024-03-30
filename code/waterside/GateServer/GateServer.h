#pragma once

#include "GatePrerequisites.h"
#include "ServerBase.h"
#include "DispatchManager.h"

namespace waterside
{
	class GateServer : public ServerBase
	{
		using super = ServerBase;
	public:
		GateServer();

		virtual ~GateServer() = default;

		virtual bool loadConfig() override;
		virtual bool initialize() override;
		virtual void release() override;

		async_simple::coro::Lazy<void> testcall();

	protected:
		virtual void onRegistHandler() override;

		virtual void runonce(float deltaTime, double currentTime) override;

		virtual int32_t getServerId() const override;
		virtual std::string_view getServiceName() const override;
		virtual float getLogicFps() override;
		virtual float getLogicFpsWarning() override;
		virtual uint16_t getNetListenPort() override;
		virtual size_t getNetMultiThreading() override;

	private:
		DispatchManager mDispatchManager;
	};
}
