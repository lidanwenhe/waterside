#include "NameRegServer.h"
#include "NameRegConfigManager.h"
#include "NameRegManager.h"

namespace waterside
{
	NameRegServer::NameRegServer()
	{
	}

	bool NameRegServer::loadConfig()
	{
		if (mCfgPath.empty())
			mCfgPath = "../../config/NameRegConfig.json";
		if (!NameRegConfigManager::instance().initialize(mCfgPath.c_str()))
			return false;

		return super::loadConfig();
	}

	bool NameRegServer::initialize()
	{
		if (!super::initialize())
			return false;

		return true;
	}

	void NameRegServer::onRegistHandler()
	{
		NameRegManager::instance().onRegistHandler(mpNetwork.get());
	}

	void NameRegServer::runonce(float deltaTime, double currentTime)
	{
		super::runonce(deltaTime, currentTime);
	}

	void NameRegServer::release()
	{
		super::release();
	}

	int32_t NameRegServer::getServerId() const
	{
		return NameRegConfigManager::instance().getServerId();
	}

	std::string_view NameRegServer::getServiceName() const
	{
		return NameRegConfigManager::instance().getServiceName();
	}

	float NameRegServer::getLogicFps()
	{
		auto cfg = NameRegConfigManager::instance().config();
		if (cfg->logic_fps() > 0.f)
			return 1.f / cfg->logic_fps();
		else
			return 0.1f;
	}

	float NameRegServer::getLogicFpsWarning()
	{
		auto cfg = NameRegConfigManager::instance().config();
		if (cfg->logic_fps_warning() > 0.f)
			return 1.f / cfg->logic_fps_warning();
		else
			return 0.2f;
	}

	uint16_t NameRegServer::getNetListenPort()
	{
		return NameRegConfigManager::instance().config()->network()->port();
	}

	size_t NameRegServer::getNetMultiThreading()
	{
		size_t x = NameRegConfigManager::instance().config()->network()->multi_threading();
		if (x == 0)
			return std::thread::hardware_concurrency();
		else
			return x;
	}
}
