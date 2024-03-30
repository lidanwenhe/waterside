#pragma once

#include "ConfigLoader.h"
#include "GameConfig_generated.h"

namespace waterside
{
	class GameConfigManager : public TLazySingleton<GameConfigManager>
	{
	public:
		GameConfigManager() = default;

		~GameConfigManager() = default;

		bool initialize(const char* path)
		{
			if (mConfig.parse("../../config/GameConfig.fbs", path))
			{
				return true;
			}
			return false;
		}

		const GameConfig* config() const
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
		TConfigLoader<GameConfig> mConfig;
	};
}
