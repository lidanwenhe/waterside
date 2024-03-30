#pragma once

#include "NameRegPrerequisites.h"
#include "ServerBase.h"

namespace waterside
{
	class NameRegServer : public ServerBase
	{
		using super = ServerBase;
	public:
		NameRegServer();

		virtual ~NameRegServer() = default;

		virtual bool loadConfig() override;
		virtual bool initialize() override;
		virtual void release() override;

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
	};
}
