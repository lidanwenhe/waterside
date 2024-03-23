#pragma once

#include "MysqlPrerequisites.h"

namespace waterside
{
	class MysqlPrepareStmt final
	{
	public:
		MysqlPrepareStmt();

		virtual ~MysqlPrepareStmt();

		vector<MysqlStmtArgs> params;
		vector<MysqlStmtArgs> results;

		void initialize(int32_t index, std::string_view sql, const vector<MysqlStmtBindArgs>& vParams, const vector<MysqlStmtBindArgs>& vResults);

		void connect(MYSQL* pMysql);

		void close();

		bool isvalid() const
		{
			return nullptr != mpStmt;
		}

		virtual void execute();

		virtual bool fetch();

		virtual void clear();

		virtual uint64_t getInsertId();

		virtual uint64_t getAffectedRows();

	private:
		void bindComplete();

		void bindParam();

		void bindResult();

		// 计算缓存大小，和相关偏移值
		size_t calcBufferSize(const vector<MysqlStmtBindArgs>& vParams, const vector<MysqlStmtBindArgs>& vResults);

		void callBindItem(size_t& offset, size_t& nAlignment, const MysqlStmtBindArgs& arg, MYSQL_BIND& item);

		void boundParams();
		void storeResults();

	private:
		MYSQL_STMT* mpStmt;
		int32_t mIndex;
		string mSql;
		bool mbProcess;
		vector<MYSQL_BIND> mBindParams;
		vector<MYSQL_BIND> mBindResults;
		vector<uint8_t> mBuffer;
	};
}
