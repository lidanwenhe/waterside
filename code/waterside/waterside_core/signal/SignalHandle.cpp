#include "SignalHandle.h"

namespace waterside
{
	void SignalHandle::nextId()
	{
		static std::atomic<uint64_t> sId = 0;
		mId = sId.fetch_add(1, std::memory_order::relaxed);;
		if (0 == mId)
		{
			mId = sId.fetch_add(1, std::memory_order::relaxed);
		}
	}
}
