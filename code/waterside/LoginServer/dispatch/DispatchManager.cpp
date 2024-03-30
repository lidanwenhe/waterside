#include "DispatchManager.h"

namespace waterside
{
	void DispatchManager::onRegistHandler(NetworkBase* pNetwork)
	{
		loginDispatch.onRegistHandler(pNetwork);
	}
}
