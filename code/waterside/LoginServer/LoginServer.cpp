#include "LoginServer.h"
#include "LoginConfigManager.h"
#include "Router.h"

namespace waterside
{
	LoginServer::LoginServer()
	{
	}

	bool LoginServer::loadConfig()
	{
		if (mCfgPath.empty())
			mCfgPath = "../../config/LoginConfig.json";
		if (!LoginConfigManager::instance().initialize(mCfgPath.c_str()))
			return false;

		return super::loadConfig();
	}

	async_simple::coro::Lazy<int> testxx(int x)
	{
		co_await LoginMysqlManager::instance()->query<onTestMysql>();
		co_return x + 1;
	}

	bool LoginServer::initialize()
	{
		auto cfgLoginDB = LoginConfigManager::instance().config()->login_db();
		mpLoginMysqlManager = std::make_unique<LoginMysqlManager>(cfgLoginDB->url()->string_view(), cfgLoginDB->multi_threading(), cfgLoginDB->reconnect_delay_second());
		if (!mpLoginMysqlManager->initialize())
			return false;

		if (!super::initialize())
			return false;
		
		Router::instance().startNameReg().start([this](async_simple::Try<bool> Result) {
			if (Result.hasError())
			{
				std::exception_ptr error = Result.getException();
				try
				{
					std::rethrow_exception(error);
				}
				catch (const std::exception& e)
				{
					LOG_ERROR("Error Happened in task. {}", e.what());
				}
			}

			if (!Result.value())
			{
				mbRun = false;
				if (mpNetwork)
				{
					mpNetwork->asyncClose();
				}
			}
		});

		return true;
	}

	void LoginServer::onRegistHandler()
	{
		auto cfg = LoginConfigManager::instance().config();
		Router::instance().initialize(mpNetwork,
			cfg->service_name()->string_view(),
			cfg->network()->ip()->string_view(),
			std::to_string(cfg->network()->port()),
			cfg->namereg_ip()->string_view(),
			cfg->namereg_port()->string_view(),
			cfg->server_id()
		);

		mpNetwork->registHandler<testxx>();

		mpLoginMysqlManager->registHandler<onTestMysql>();

	}

	void LoginServer::runonce(float deltaTime, double currentTime)
	{
		mpLoginMysqlManager->onProcessPacket();
		super::runonce(deltaTime, currentTime);
	}

	void LoginServer::release()
	{
		if (mpLoginMysqlManager)
			mpLoginMysqlManager->release();
		super::release();
	}

	int32_t LoginServer::getServerId() const
	{
		return LoginConfigManager::instance().getServerId();
	}

	string_view LoginServer::getServiceName() const
	{
		return LoginConfigManager::instance().getServiceName();
	}

	float LoginServer::getLogicFps()
	{
		auto cfg = LoginConfigManager::instance().config();
		if (cfg->logic_fps() > 0.f)
			return 1.f / cfg->logic_fps();
		else
			return 0.1f;
	}

	float LoginServer::getLogicFpsWarning()
	{
		auto cfg = LoginConfigManager::instance().config();
		if (cfg->logic_fps_warning() > 0.f)
			return 1.f / cfg->logic_fps_warning();
		else
			return 0.2f;
	}

	uint16_t LoginServer::getNetListenPort()
	{
		return LoginConfigManager::instance().config()->network()->port();
	}

	size_t LoginServer::getNetMultiThreading()
	{
		size_t x = LoginConfigManager::instance().config()->network()->multi_threading();
		if (x == 0)
			return std::thread::hardware_concurrency();
		else
			return x;
	}
}