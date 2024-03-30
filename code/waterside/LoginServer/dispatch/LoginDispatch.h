#pragma once

#include "LoginPrerequisites.h"
#include "message_login_generated.h"

namespace waterside
{
	class LoginDispatch : private boost::noncopyable
	{
	public:
		LoginDispatch() = default;

		~LoginDispatch() = default;

		void onRegistHandler(NetworkBase* pNetwork);

	protected:
		async_simple::coro::Lazy<void> onAccountLogin(SessionID sessionId, const AccountLogin* fb);
	};
}
