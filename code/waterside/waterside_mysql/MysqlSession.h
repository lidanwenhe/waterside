#pragma once

#include "MysqlPrepareStmt.h"

namespace waterside
{
	class MysqlSession final
	{
	public:
		MysqlSession();

		~MysqlSession();

		void initialize(std::string_view url);

		void connect();

		bool ping();

		void clear();

		void registerStmt(int32_t index, std::string_view sql, const vector<MysqlStmtBindArgs>& vParams, const vector<MysqlStmtBindArgs>& vResults);

		MysqlPrepareStmt* findStmt(int32_t index);
		

		class lock_guard final
		{
		public:
			lock_guard(MysqlSession& db)
				: mDB(db)
				, mbcommit(false)
			{
				mDB.startTransaction();
			}
			~lock_guard()
			{
				mbcommit ? mDB.commit() : mDB.rollback();
			}

			void commit(bool bcommit = true) { mbcommit = bcommit; }

		private:
			MysqlSession& mDB;
			bool mbcommit;
		};

	protected:
		void startTransaction();
		void commit();
		void rollback();

		void close();

	private:
		string mUser;
		string mPassword;
		string mHost;
		uint16_t mPort;
		string mDB;

		MYSQL* mpMysql;

		std::unordered_map<int32_t, MysqlPrepareStmt> mStmt;
		MysqlPrepareStmt* mpCurStmt;
	};
}