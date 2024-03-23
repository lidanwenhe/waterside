#pragma once

#include "MysqlManager.h"

namespace waterside
{
	class LoginMysqlManager : public MysqlManager, public TSingleton<LoginMysqlManager>
	{
	public:
		LoginMysqlManager(string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond);

		virtual ~LoginMysqlManager() = default;

	protected:
		virtual void onRegisterStmt(MysqlSession* pSession) override;

	private:
	};

	void onTestMysql(MysqlSession* pSession);
}
