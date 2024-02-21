#pragma once

#include "prerequisites.h"
#include "tbb/concurrent_hash_map.h"

namespace waterside
{
	class SessionIdManager
	{
	public:
		SessionIdManager();

		SessionID getNewId();

		void recyclingId(SessionID id);

	private:
		std::atomic<SessionID> mNextId;
		tbb::concurrent_hash_map<SessionID, int> mSessionIds;
	};
}
