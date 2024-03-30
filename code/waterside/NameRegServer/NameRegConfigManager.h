#pragma once

#include "ConfigLoader.h"
#include "NameRegConfig_generated.h"

namespace waterside
{
	class NameRegConfigManager : public TLazySingleton<NameRegConfigManager>
	{
	public:
		NameRegConfigManager() = default;

		~NameRegConfigManager() = default;

		bool initialize(const char* path)
		{
			if (mConfig.parse("../../config/NameRegConfig.fbs", path))
			{
				return true;
			}
			return false;
		}

		const NameRegConfig* config() const
		{
			return mConfig.config();
		}

		int32_t getServerId() const
		{
			return config()->server_id();
		}

		std::string_view getServiceName() const
		{
			return config()->service_name()->string_view();
		}

	private:
		TConfigLoader<NameRegConfig> mConfig;
	};
}