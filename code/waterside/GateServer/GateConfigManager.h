#pragma once

#include "ConfigLoader.h"
#include "GateConfig_generated.h"

namespace waterside
{
	class GateConfigManager : public TLazySingleton<GateConfigManager>
	{
	public:
		GateConfigManager() = default;

		~GateConfigManager() = default;

		bool initialize(const char* path)
		{
			if (mConfig.parse("../../config/GateConfig.fbs", path))
			{
				return true;
			}
			return false;
		}

		const GateConfig* config() const
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
		TConfigLoader<GateConfig> mConfig;
	};
}
