#pragma once

#include "MysqlPrerequisites.h"

namespace waterside
{
	class MysqlPrepareStmt : private boost::noncopyable
	{
	public:
		MysqlPrepareStmt();

		virtual ~MysqlPrepareStmt();

		virtual void initialize() {}

		void connect(MYSQL* pMysql);

		void close();

		bool isvalid() const
		{
			return nullptr != mpStmt;
		}

		void execute();

		bool fetch();

		void clear();

		uint64_t getInsertId();

		uint64_t getAffectedRows();

	private:
		void bindComplete();

		void bindParam();

		void bindResult();

		// 计算缓存大小，和相关偏移值
		size_t _calcBufferSize(const std::vector<MysqlStmtBindArgs>& vParams, const std::vector<MysqlStmtBindArgs>& vResults);

		void _callBindItem(size_t& offset, size_t& nAlignment, const MysqlStmtBindArgs& arg, MYSQL_BIND& item);

	protected:
		void _initialize(uint32_t index, std::string_view sql, const std::vector<MysqlStmtBindArgs>& vParams, const std::vector<MysqlStmtBindArgs>& vResults);

		void _setParam(size_t i, std::string_view name, int8_t value);
		void _setParam(size_t i, std::string_view name, uint8_t value);
		void _setParam(size_t i, std::string_view name, int16_t value);
		void _setParam(size_t i, std::string_view name, uint16_t value);
		void _setParam(size_t i, std::string_view name, int32_t value);
		void _setParam(size_t i, std::string_view name, uint32_t value);
		void _setParam(size_t i, std::string_view name, int64_t value);
		void _setParam(size_t i, std::string_view name, uint64_t value);
		void _setParam(size_t i, std::string_view name, float value);
		void _setParam(size_t i, std::string_view name, double value);
		void _setParam(size_t i, std::string_view name, std::string_view value);

		void _getResult(size_t i, std::string_view name, int8_t& value);
		void _getResult(size_t i, std::string_view name, uint8_t& value);
		void _getResult(size_t i, std::string_view name, int16_t& value);
		void _getResult(size_t i, std::string_view name, uint16_t& value);
		void _getResult(size_t i, std::string_view name, int32_t& value);
		void _getResult(size_t i, std::string_view name, uint32_t& value);
		void _getResult(size_t i, std::string_view name, int64_t& value);
		void _getResult(size_t i, std::string_view name, uint64_t& value);
		void _getResult(size_t i, std::string_view name, float& value);
		void _getResult(size_t i, std::string_view name, double& value);
		void _getResult(size_t i, std::string_view name, std::string_view& value);

	private:
		MYSQL_STMT* mpStmt;
		uint32_t mIndex;
		std::string mSql;
		bool mbProcess;
		std::vector<MYSQL_BIND> mBindParams;
		std::vector<MYSQL_BIND> mBindResults;
		std::vector<uint8_t> mBuffer;
	};
}
