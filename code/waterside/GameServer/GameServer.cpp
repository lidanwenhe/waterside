#include "GameServer.h"
#include "GameConfigManager.h"
#include "Router.h"

namespace waterside
{
	GameServer::GameServer()
	{
	}

	bool GameServer::loadConfig()
	{
		if (mCfgPath.empty())
			mCfgPath = "../../config/GameConfig.json";
		if (!GameConfigManager::instance().initialize(mCfgPath.c_str()))
			return false;

		return super::loadConfig();
	}

	async_simple::coro::Lazy<int> testxx(int x);
	async_simple::coro::Lazy<void> GameServer::testcall()
	{
		auto xx = co_await Router::instance().call<testxx>("Login"sv, 100);
		
		int x = xx.value();
		++x;
		co_return;
	}

	bool GameServer::initialize()
	{
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
			else
			{
				string_view s = "Login"sv;

				//auto xx = mpNetwork->call<testxx>(111, 100)
				testcall().start([this](async_simple::Try<void> Result) {
					});

				//auto r = Router::instance().call<testxx>("Login", 100);
				//if (r)
				{
					//
				}
			}
			});

		return true;
	}

	void GameServer::onRegistHandler()
	{
		auto cfg = GameConfigManager::instance().config();
		Router::instance().initialize(mpNetwork,
			cfg->service_name()->string_view(),
			cfg->network()->ip()->string_view(),
			std::to_string(cfg->network()->port()),
			cfg->namereg_ip()->string_view(),
			cfg->namereg_port()->string_view(),
			cfg->server_id()
		);
	}

	void GameServer::runonce(float deltaTime, double currentTime)
	{
		super::runonce(deltaTime, currentTime);
	}

	void GameServer::release()
	{
		super::release();
	}

	int32_t GameServer::getServerId() const
	{
		return GameConfigManager::instance().getServerId();
	}

	string_view GameServer::getServiceName() const
	{
		return GameConfigManager::instance().getServiceName();
	}

	float GameServer::getLogicFps()
	{
		auto cfg = GameConfigManager::instance().config();
		if (cfg->logic_fps() > 0.f)
			return 1.f / cfg->logic_fps();
		else
			return 0.1f;
	}

	float GameServer::getLogicFpsWarning()
	{
		auto cfg = GameConfigManager::instance().config();
		if (cfg->logic_fps_warning() > 0.f)
			return 1.f / cfg->logic_fps_warning();
		else
			return 0.2f;
	}

	uint16_t GameServer::getNetListenPort()
	{
		return GameConfigManager::instance().config()->network()->port();
	}

	size_t GameServer::getNetMultiThreading()
	{
		size_t x = GameConfigManager::instance().config()->network()->multi_threading();
		if (x == 0)
			return std::thread::hardware_concurrency();
		else
			return x;
	}
}