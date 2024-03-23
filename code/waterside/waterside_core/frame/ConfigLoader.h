#pragma once

#include "prerequisites.h"

namespace waterside
{
	class ConfigLoader
	{
	public:
		ConfigLoader() = default;
		virtual ~ConfigLoader() = 0 {};

		virtual bool parse(const char* schemafilename, const char* jsonfilename);

	protected:
		flatbuffers::Parser mParser;
	};

	template<typename T>
	class TConfigLoader final : public ConfigLoader, public TSingleton<TConfigLoader<T>>
	{
	public:
		TConfigLoader()
			: mConfig(nullptr)
		{
		}

		virtual ~TConfigLoader() = default;

		virtual bool parse(const char* schemafilename, const char* jsonfilename) override
		{
			if (!ConfigLoader::parse(schemafilename, jsonfilename))
				return false;

			flatbuffers::Verifier verifier(mParser.builder_.GetBufferPointer(), mParser.builder_.GetSize());
			if (!verifier.VerifyBuffer<T>(nullptr))
			{
				printf("verify schemafilename:%s, jsonfilename:%s failed!\n", schemafilename, jsonfilename);
				return false;
			}

			mConfig = flatbuffers::GetRoot<T>(mParser.builder_.GetBufferPointer());
			return true;
		}

		const T* config() const
		{
			return mConfig;
		}

	private:
		const T* mConfig;
	};
}
