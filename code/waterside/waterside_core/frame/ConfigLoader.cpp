#include "ConfigLoader.h"

namespace waterside
{
	bool ConfigLoader::parse(const char* schemafilename, const char* jsonfilename)
	{
		// load FlatBuffer schema (.fbs) and JSON from disk
		std::string schemafile;
		std::string jsonfile;
		bool ok = flatbuffers::LoadFile(schemafilename, false, &schemafile) &&
			flatbuffers::LoadFile(jsonfilename, false, &jsonfile);
		if (!ok)
		{
			printf("couldn't load schemafilename:%s, jsonfilename:%s file!\n", schemafilename, jsonfilename);
			return false;
		}

		// parse schema first, so we can use it to parse the data after
		const char* include_directories[] = { "../../config", nullptr};
		ok = mParser.Parse(schemafile.c_str(), include_directories) &&
			mParser.Parse(jsonfile.c_str(), include_directories);
		if (!ok)
		{
			printf("parse schemafilename:%s, jsonfilename:%s failed: %s!\n", schemafilename, jsonfilename, mParser.error_.c_str());
			return false;
		}

		// here, parser.builder_ contains a binary buffer that is the parsed data.
		return true;
	}
}