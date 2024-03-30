#include "LoginDispatch.h"
#include "message_id_generated.h"
#include "LoginMysqlManager.h"

namespace waterside
{
	void LoginDispatch::onRegistHandler(NetworkBase* pNetwork)
	{
		pNetwork->registerDispatcher(NEW_CORO_NETWORK_DISPATCHER(AccountLogin, MESSAGE_ID_ACCOUNT_LOGIN, onAccountLogin));
	}

	async_simple::coro::Lazy<void> LoginDispatch::onAccountLogin(SessionID sessionId, const AccountLogin* fb)
	{
		auto acc = fb->account()->string_view();

		auto ret = co_await LoginMysqlManager::instance()->query<&LoginMysqlManager::onAccountLogin>(acc);
		if (ret)
		{
			auto userid = std::get<0>(ret.value());
		}
		co_return;
	}

}