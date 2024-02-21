#pragma once

#include "prerequisites.h"

namespace waterside
{
	class SignalHandle final
	{
	public:
		SignalHandle()
			: mId(0)
		{
		}

		bool isValid() const
		{
			return 0 != mId;
		}

		void nextId();

		bool operator == (const SignalHandle& other) const
		{
			return mId == other.mId;
		}

		bool operator != (const SignalHandle& other) const
		{
			return mId != other.mId;
		}

	protected:
		uint64_t mId;
	};
}
