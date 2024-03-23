#pragma once

#include "NameRegPrerequisites.h"
#include "RouteBase.h"

namespace waterside
{
	class NameRegManager : public RouteBase, public TLazySingleton<NameRegManager>
	{
	public:
		NameRegManager() = default;

		~NameRegManager() = default;

		void onRegistHandler(TcpNetwork* pNetwork);
	};
}