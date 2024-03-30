#include "GateServer.h"
#include "GateConfigManager.h"
#include "Router.h"

namespace waterside
{
	GateServer::GateServer()
	{
	}

	bool GateServer::loadConfig()
	{
		if (mCfgPath.empty())
			mCfgPath = "../../config/GateConfig.json";
		if (!GateConfigManager::instance().initialize(mCfgPath.c_str()))
			return false;

		return super::loadConfig();
	}

	async_simple::coro::Lazy<int> testxx(int x);
	async_simple::coro::Lazy<void> GateServer::testcall()
	{
		auto xx = co_await Router::instance().call<testxx>("Login"sv, 100);
		
		int x = xx.value();
		++x;
		co_return;
	}

	bool GateServer::initialize()
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
				std::string_view s = "Login"sv;

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

	void GateServer::onRegistHandler()
	{
		auto cfg = GateConfigManager::instance().config();
		Router::instance().initialize(mpNetwork,
			cfg->service_name()->string_view(),
			cfg->network()->ip()->string_view(),
			std::to_string(cfg->network()->port()),
			cfg->namereg_ip()->string_view(),
			cfg->namereg_port()->string_view(),
			cfg->server_id()
		);

		mDispatchManager.onRegistHandler(mpNetwork.get());
	}

	void GateServer::runonce(float deltaTime, double currentTime)
	{
		super::runonce(deltaTime, currentTime);
	}

	void GateServer::release()
	{
		super::release();
	}

	int32_t GateServer::getServerId() const
	{
		return GateConfigManager::instance().getServerId();
	}

	std::string_view GateServer::getServiceName() const
	{
		return GateConfigManager::instance().getServiceName();
	}

	float GateServer::getLogicFps()
	{
		auto cfg = GateConfigManager::instance().config();
		if (cfg->logic_fps() > 0.f)
			return 1.f / cfg->logic_fps();
		else
			return 0.1f;
	}

	float GateServer::getLogicFpsWarning()
	{
		auto cfg = GateConfigManager::instance().config();
		if (cfg->logic_fps_warning() > 0.f)
			return 1.f / cfg->logic_fps_warning();
		else
			return 0.2f;
	}

	uint16_t GateServer::getNetListenPort()
	{
		return GateConfigManager::instance().config()->network()->port();
	}

	size_t GateServer::getNetMultiThreading()
	{
		size_t x = GateConfigManager::instance().config()->network()->multi_threading();
		if (x == 0)
			return std::thread::hardware_concurrency();
		else
			return x;
	}
}