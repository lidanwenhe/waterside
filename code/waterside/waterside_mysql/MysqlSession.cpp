#include "MysqlSession.h"
#include "MysqlException.h"

namespace waterside
{
	MysqlSession::MysqlSession()
		: mPort(0)
		, mpMysql(nullptr)
		, mpCurStmt(nullptr)
	{
	}

	MysqlSession::~MysqlSession()
	{
		clear();

		mStmt.clear();

		if (mpMysql)
		{
			mysql_close(mpMysql);
			mpMysql = nullptr;
		}
	}

	void MysqlSession::initialize(std::string_view url)
	{
		if (auto i = url.find("mysql://"); i != std::string_view::npos)
		{
			url = url.substr(strlen("mysql://"));
		}

		if (auto i = url.find(":"); i != std::string_view::npos)
		{
			mUser = url.substr(0, i);
			url = url.substr(i + 1);
		}
		else
		{
			MLOG_ERROR(MYSQL, "mysql url={} user not found"sv, url);
		}
		if (auto i = url.find("@"); i != std::string_view::npos)
		{
			mPassword = url.substr(0, i);
			url = url.substr(i + 1);
		}
		else
		{
			MLOG_ERROR(MYSQL, "mysql url={} password not found"sv, url);
		}
		if (auto i = url.find("/"); i != std::string_view::npos)
		{
			mDB = url.substr(i + 1);
			url = url.substr(0, i);
		}
		else
		{
			MLOG_ERROR(MYSQL, "mysql url={} db not found"sv, url);
		}
		if (auto i = url.find(":"); i != std::string_view::npos)
		{
			mHost = url.substr(0, i);
			url = url.substr(i + 1);
			std::from_chars(url.data(), url.data() + url.size(), mPort);
		}
		else
		{
			mHost = url;
			mPort = 3306;
		}
	}

	void MysqlSession::connect()
	{
		close();

		mpMysql = mysql_init(nullptr);
		if (!mpMysql)
		{
			MYSQL_DB_THROW("init mysql fail"sv);
		}

		if (nullptr == mysql_real_connect(mpMysql, mHost.c_str(), mUser.c_str(), mPassword.c_str(), mDB.c_str(), mPort, nullptr, CLIENT_MULTI_STATEMENTS))
		{
			MYSQL_DB_THROW(mysql_errno(mpMysql), mysql_error(mpMysql), mysql_sqlstate(mpMysql), "mysql_real_connect fail"sv);
		}

		if (0 != mysql_set_character_set(mpMysql, "utf8mb4"))
		{
			MYSQL_DB_THROW(mysql_errno(mpMysql), mysql_error(mpMysql), mysql_sqlstate(mpMysql), "New client character set: {}"sv, mysql_character_set_name(mpMysql));
		}

		for (auto& p : mStmt)
		{
			p->connect(mpMysql);
		}
	}

	void MysqlSession::close()
	{
		clear();

		if (mpMysql)
		{
			mysql_close(mpMysql);
			mpMysql = nullptr;
		}
	}

	bool MysqlSession::ping()
	{
		if (nullptr != mpMysql)
		{
			if (0 != mysql_ping(mpMysql))
			{
				int r = mysql_errno(mpMysql);
				switch (r)
				{
				case CR_SERVER_GONE_ERROR:
				case CR_SERVER_LOST:
					return false;
				}

				MYSQL_DB_THROW(r, mysql_error(mpMysql), mysql_sqlstate(mpMysql), "mysql_ping fail"sv);
			}

			return true;
		}
		return false;
	}

	void MysqlSession::clear()
	{
		if (mpCurStmt)
		{
			mpCurStmt->clear();
			mpCurStmt = nullptr;
		}
	}

	void MysqlSession::registerStmt(std::unique_ptr<MysqlPrepareStmt>&& pstmt)
	{
		pstmt->initialize();
		mStmt.emplace_back(std::move(pstmt));
	}

	MysqlPrepareStmt* MysqlSession::findStmt(uint32_t index)
	{
		if (index >= mStmt.size())
		{
			return nullptr;
		}

		MysqlPrepareStmt* pStmt = mStmt[index].get();
		if (!pStmt->isvalid())
		{
			return nullptr;
		}

		if (mpCurStmt)
		{
			mpCurStmt->clear();
		}
		mpCurStmt = pStmt;
		return mpCurStmt;
	}

	void MysqlSession::startTransaction()
	{
		//if (0 != mysql_query(mpMysql, "START TRANSACTION"))
		//{
		//	MYSQL_DB_THROW(mysql_errno(mpMysql), mysql_error(mpMysql), mysql_sqlstate(mpMysql), "START TRANSACTION fail"sv);
		//}
		mysql_autocommit(mpMysql, false);
	}

	void MysqlSession::commit()
	{
		mysql_commit(mpMysql);
		mysql_autocommit(mpMysql, true);
	}

	void MysqlSession::rollback()
	{
		mysql_rollback(mpMysql);
		mysql_autocommit(mpMysql, true);
	}
}
