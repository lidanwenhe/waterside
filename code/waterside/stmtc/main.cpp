#include "StmtFile.h"

int main(int argc, char* argv[])
{
	if (argc == 3)
	{
		StmtFile file;
		if (file.parse(argv[1]))
		{
			file.gen(argv[2]);
		}
	}
	else
	{
		printf(std::format("cmd error, help:\nfilename, path\n").c_str());
	}
	return 0;
}
