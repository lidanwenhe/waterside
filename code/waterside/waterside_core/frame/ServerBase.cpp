#include "ServerBase.h"
#include "Logger.h"
#include "SnowflakeId.h"
#include "TimerManager.h"

namespace waterside
{
	ServerBase::ServerBase()
		: mbDaemon(false)
		, mbRun(true)
	{
		srand((unsigned int)time(nullptr));
	}

#ifndef _MSC_VER
	static void startDaemon()
	{
		pid_t pid = fork();
		if (pid != 0)
		{
			// 结束父进程
			exit(0);
		}

		// 建立一个新的进程组，在这个新的进程组中，子进程成为这个进程组的首进程
		setsid();

		signal(SIGHUP, SIG_IGN);
#ifndef _DEBUG
		signal(SIGINT, SIG_IGN);
#endif
		signal(SIGQUIT, SIG_IGN);
		signal(SIGPIPE, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		signal(SIGTERM, SIG_IGN);
		signal(SIGCHLD, SIG_IGN);

		// close stdio
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	}
#endif

	void ServerBase::startup(int argc, char* argv[])
	{
		if (argc > 1)
		{
			for (int i = 1; i < argc; i++)
			{
				if (strcmp(argv[i], "-daemon") == 0)
				{
					mbDaemon = true;
				}
				else
				{
					mCfgPath = argv[i];
				}
			}
		}

#ifdef __GNUC__
		if (mbDaemon)
		{
			printf("~~~~~~~~~~~~~~~~~ start daemon\n");

			startDaemon();
		}
#endif

		try
		{
			if (loadConfig() && initialize())
			{
				run();
			}
		}
		catch (const std::exception& e)
		{
			LOG_ERROR(e.what());
		}
		catch (...)
		{
			LOG_ERROR("c++ unknown exception");
		}

		release();
	}

	void ServerBase::initLogger()
	{
		mLoggerName = getServiceName();
		TLogger<>::instance().init(mLoggerName);
		TLogger<LOGGER_CATEGORY_NET>::instance().init(mLoggerName + "-Net");
	}

	void ServerBase::releaseLogger()
	{
		TLogger<>::instance().join();
		TLogger<LOGGER_CATEGORY_NET>::instance().join();
	}

	bool ServerBase::loadConfig()
	{
		initLogger();

		if (!SnowflakeId::instance().initialize(getServerId()))
			return false;

		TimerManager::instance().initialize();
		return true;
	}

	bool ServerBase::initialize()
	{
		mpNetwork = std::make_shared<TcpNetwork>(getNetMultiThreading(), boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), getNetListenPort()));
		onRegistHandler();

		mpNetwork->start([this]() {
			mbRun = false;
			}).start([](async_simple::Try<void> Result) {
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
				});

		return true;
	}

	void ServerBase::run()
	{
		float fpsTime = getLogicFps();
		float fpsWarningTime = getLogicFpsWarning();

		auto last = std::chrono::system_clock::now();
		float deltaTime = 0.f;
		while (mbRun)
		{
			auto currentTime = std::chrono::duration_cast<std::chrono::duration<double>>(last.time_since_epoch()).count();
			
			runonce(deltaTime, currentTime);

			// 逻辑也会耗时间，重新计算时间
			auto curr = std::chrono::system_clock::now();
			deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(curr - last).count();

			if (deltaTime < fpsTime)
			{
				std::chrono::duration<float> dely(fpsTime - deltaTime);
				std::this_thread::sleep_for(dely);

				curr += std::chrono::duration_cast<std::chrono::microseconds>(dely);
			}
			else if (deltaTime > fpsWarningTime)
			{
				LOG_ERROR("logic tick time out: {}", deltaTime);
			}

			deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(curr - last).count();
			last = curr;
		}
	}

	void ServerBase::runonce(float deltaTime, double currentTime)
	{
		mpNetwork->onProcessPacket();
		TimerManager::instance().tick();
	}

	void ServerBase::release()
	{
		if (mpNetwork)
			mpNetwork->release();
		TimerManager::instance().release();
		releaseLogger();
	}
}
