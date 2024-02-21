#include "SessionIdManager.h"

namespace waterside
{
	SessionIdManager::SessionIdManager()
		: mNextId(0)
	{
		std::default_random_engine re(std::random_device{}());
		mNextId = std::uniform_int_distribution<>{}(re);
	}

	SessionID SessionIdManager::getNewId()
	{
		SessionID id = 0;
		do
		{
			id = mNextId.fetch_add(1, std::memory_order::relaxed);
			if (id == 0)
			{
				id = mNextId.fetch_add(1, std::memory_order::relaxed);
			}
		} while (!mSessionIds.insert(std::make_pair(id, 0)));
		return id;
	}

	void SessionIdManager::recyclingId(SessionID conv)
	{
		mSessionIds.erase(conv);
	}
}