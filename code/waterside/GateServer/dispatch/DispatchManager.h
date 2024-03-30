#pragma once

#include "LoginDispatch.h"

namespace waterside
{
	class DispatchManager : public TSingleton<DispatchManager>
	{
	public:
		DispatchManager() = default;

		~DispatchManager() = default;

		void onRegistHandler(NetworkBase* pNetwork);

		LoginDispatch loginDispatch;
	};
}
