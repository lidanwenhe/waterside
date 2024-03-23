#include "GameServer.h"

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	waterside::GameServer theServer;
	theServer.startup(argc, argv);
}
