// create by stmtc
#include "LoginMysqlManagerBase.h"

namespace waterside {
void StmtAccountLogin::initialize()
{
	_initialize(ID,
		"CALL `account_login`(?);",
		{
			{MYSQL_STMT_TYPE::VARSTRING, 0},
		},
		{
			{MYSQL_STMT_TYPE::INT},
		});
}

void StmtAccountLogin::boundParams(std::string_view account)
{
	set_account(account);
}

LoginMysqlManagerBase::LoginMysqlManagerBase(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond)
	: MysqlManager(url, multithreading, reconnectDelaySecond)
{
}

void LoginMysqlManagerBase::onRegisterStmt(MysqlSession* pSession)
{
	pSession->registerStmt(std::make_unique<StmtAccountLogin>());
}

void LoginMysqlManagerBase::onRegistHandler()
{
	registHandler<&LoginMysqlManagerBase::onAccountLogin>();
}

std::tuple<int32_t> LoginMysqlManagerBase::onAccountLogin(MysqlSession* pSession, std::string_view account)
{
	std::tuple<int32_t> ret;
	auto pstmt = pSession->findStmt<StmtAccountLogin>();
	if (pstmt)
	{
		pstmt->boundParams(account);
		pstmt->execute();
		if (pstmt->fetch())
		{
			std::get<0>(ret) = pstmt->get_userid();
		}
	}
	else
	{
		assert(false);
	}
	return ret;
}
}
