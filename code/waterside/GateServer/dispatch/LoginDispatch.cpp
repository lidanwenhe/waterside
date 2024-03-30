#include "LoginDispatch.h"
#include "message_id_generated.h"

namespace waterside
{
	void LoginDispatch::onRegistHandler(NetworkBase* pNetwork)
	{
		pNetwork->registerDispatcher(NEW_CORO_NETWORK_DISPATCHER(AccountLogin, MESSAGE_ID_ACCOUNT_LOGIN, onAccountLogin));
	}

	async_simple::coro::Lazy<void> LoginDispatch::onAccountLogin(SessionID sessionId, const AccountLogin* fb)
	{
		auto acc = fb->account()->string_view();
		co_return;
	}

}