#pragma once

#include "ConfigLoader.h"
#include "LoginConfig_generated.h"

namespace waterside
{
	class LoginConfigManager : public TLazySingleton<LoginConfigManager>
	{
	public:
		LoginConfigManager() = default;

		~LoginConfigManager() = default;

		bool initialize(const char* path)
		{
			if (mConfig.parse("../../config/LoginConfig.fbs", path))
			{
				return true;
			}
			return false;
		}

		const LoginConfig* config() const
		{
			return mConfig.config();
		}

		int32_t getServerId() const
		{
			return config()->server_id();
		}

		string_view getServiceName() const
		{
			return config()->service_name()->string_view();
		}

	private:
		TConfigLoader<LoginConfig> mConfig;
	};
}
