#pragma once

#include "LoginMysqlManagerBase.h"

namespace waterside
{
	class LoginMysqlManager final : public LoginMysqlManagerBase, public TSingleton<LoginMysqlManager>
	{
		using super = LoginMysqlManagerBase;
	public:
		LoginMysqlManager(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond);

		virtual ~LoginMysqlManager() = default;

	protected:
		virtual void onRegisterStmt(MysqlSession* pSession) override;

	public:
		virtual void onRegistHandler() override;

		void onAccountLoin(MysqlSession* pSession, std::string_view account);

	};
}
