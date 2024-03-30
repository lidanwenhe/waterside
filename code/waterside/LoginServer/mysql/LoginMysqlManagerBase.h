// create by stmtc
#pragma once

#include "MysqlManager.h"

namespace waterside {
struct StmtAccountLogin final: public MysqlPrepareStmt
{
	StmtAccountLogin() = default;
	virtual ~StmtAccountLogin() = default;
	enum { ID = 0 };
	virtual void initialize() override;
	void boundParams(std::string_view account);
	void set_account(std::string_view value) { _setParam(0, "account", value); }
	int32_t get_userid() { int32_t value; _getResult(0, "userid", value); return value; }
};

class LoginMysqlManagerBase : public MysqlManager
{
public:
	LoginMysqlManagerBase(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond);
	virtual ~LoginMysqlManagerBase() = default;
protected:
	virtual void onRegisterStmt(MysqlSession* pSession) override;
public:
	virtual void onRegistHandler();
	std::tuple<int32_t> onAccountLogin(MysqlSession* pSession, std::string_view account);
};
}
