#pragma once

#include "LoginPrerequisites.h"
#include "ServerBase.h"
#include "LoginMysqlManager.h"
#include "DispatchManager.h"

namespace waterside
{
	class LoginServer : public ServerBase, private MysqlLibrary
	{
		using super = ServerBase;
	public:
		LoginServer();

		virtual ~LoginServer() = default;

		virtual bool loadConfig() override;
		virtual bool initialize() override;
		virtual void release() override;

	protected:
		virtual void initLogger() override;
		virtual void releaseLogger() override;

		virtual void onRegistHandler() override;

		void onRegistLoginDBHandler();

		virtual void runonce(float deltaTime, double currentTime) override;

		virtual int32_t getServerId() const override;
		virtual std::string_view getServiceName() const override;
		virtual float getLogicFps() override;
		virtual float getLogicFpsWarning() override;
		virtual uint16_t getNetListenPort() override;
		virtual size_t getNetMultiThreading() override;

	private:
		std::unique_ptr<LoginMysqlManager> mpLoginMysqlManager;
		DispatchManager mDispatchManager;
	};
}
