#pragma once

#include "prerequisites.h"
#include "TcpNetwork.h"

namespace waterside
{
	class ServerBase : public TSingleton<ServerBase>
	{
	public:
		ServerBase();

		virtual ~ServerBase() = default;

		// 调用初始化，然后开启线程
		void startup(int argc, char* argv[]);

		// 加载配置
		virtual bool loadConfig();

		// 初始化
		virtual bool initialize();

		// 运行
		virtual void run();

		// 释放
		virtual void release();

	protected:
		void initLogger();
		void releaseLogger();

		virtual void onRegistHandler() {}

		virtual void runonce(float deltaTime, double currentTime);

		virtual int32_t getServerId() const = 0;
		virtual string_view getServiceName() const = 0;
		virtual float getLogicFps() = 0;
		virtual float getLogicFpsWarning() = 0;
		virtual uint16_t getNetListenPort() = 0;
		virtual size_t getNetMultiThreading() = 0;

	protected:
		string mCfgPath;
		string mLoggerName;
		bool mbDaemon;

		volatile bool mbRun;

		std::shared_ptr<TcpNetwork> mpNetwork;
	};
}