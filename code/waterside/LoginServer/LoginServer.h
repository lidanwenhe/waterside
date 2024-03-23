#pragma once

#include "LoginPrerequisites.h"
#include "ServerBase.h"
#include "LoginMysqlManager.h"

namespace waterside
{
	class LoginServer : public ServerBase
	{
		using super = ServerBase;
	public:
		LoginServer();

		virtual ~LoginServer() = default;

		virtual bool loadConfig() override;
		virtual bool initialize() override;
		virtual void release() override;

	protected:
		virtual void onRegistHandler() override;

		virtual void runonce(float deltaTime, double currentTime) override;

		virtual int32_t getServerId() const override;
		virtual string_view getServiceName() const override;
		virtual float getLogicFps() override;
		virtual float getLogicFpsWarning() override;
		virtual uint16_t getNetListenPort() override;
		virtual size_t getNetMultiThreading() override;

	private:
		std::unique_ptr<LoginMysqlManager> mpLoginMysqlManager;
	};
}
