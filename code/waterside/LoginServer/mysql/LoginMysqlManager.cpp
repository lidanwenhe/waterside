#include "LoginMysqlManager.h"
#include "LoginConfigManager.h"

namespace waterside
{
	LoginMysqlManager::LoginMysqlManager(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond)
		: LoginMysqlManagerBase(url, multithreading, reconnectDelaySecond)
	{
	}

	void LoginMysqlManager::onRegisterStmt(MysqlSession* pSession)
	{
		super::onRegisterStmt(pSession);

		//FROM_UNIXTIME(NULLIF(?, 0))
		std::string_view strSql = "SELECT IFNULL(UNIX_TIMESTAMP(`login_time`), 0),IFNULL(UNIX_TIMESTAMP(`logout_time`), 0) FROM user_info_t WHERE `user_id`=?;"sv;
		//pSession->registerStmt<&LoginMysqlManager::onAccountLoin>(strSql, { {MYSQL_STMT_TYPE::LONG} }, { {MYSQL_STMT_TYPE::INT}, {MYSQL_STMT_TYPE::INT} });

	}

	void LoginMysqlManager::onRegistHandler()
	{
		super::onRegistHandler();

		registHandler<&LoginMysqlManager::onAccountLoin>();
	}

	void LoginMysqlManager::onAccountLoin(MysqlSession* pSession, std::string_view account)
	{
		/**auto pstmt = pSession->findStmt<&LoginMysqlManager::onAccountLoin>();
		if (nullptr == pstmt)
		{
			return;
		}

		unique_id uid = 1;
		pstmt->params.emplace_back(uid);
		pstmt->execute();
		if (pstmt->fetch() && pstmt->results.size() == 2)
		{
			int x = std::get<int>(pstmt->results[0]);
			int x1 = std::get<int>(pstmt->results[1]);

		}*/
	}
}