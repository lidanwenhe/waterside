#pragma once

#include "prerequisites.h"

namespace waterside
{
	enum class MYSQL_STMT_TYPE : uint8_t
	{
		BYTE,
		UBYTE,
		SHORT,
		USHORT,
		INT,
		UINT,
		LONG,
		ULONG,
		FLOAT,
		DOUBLE,
		STRING,
		VARSTRING,
		TINYBLOB,
		BLOB,
		MEDIUMBLOB,
	};

	struct MysqlStmtBindArgs final
	{
		MysqlStmtBindArgs(MYSQL_STMT_TYPE _type)
			: type(_type)
			, bNull(false)
			, size(0)
		{
		}

		MysqlStmtBindArgs(MYSQL_STMT_TYPE _type, uint32_t _size, bool _bNull = false)
			: type(_type)
			, bNull(_bNull)
			, size(_size)
		{
		}

		MYSQL_STMT_TYPE type;
		bool bNull;
		uint32_t size;
	};

	using MysqlStmtArgs = std::variant<int32_t, int64_t, float, double, std::string_view>;

	/**struct ObjectSqlData
	{
		string strLoadProps;
		vector<MysqlStmtBindArgs> vLoadBindArgs;
		string strSaveProps;
		string strSaveParams;
		vector<MysqlStmtBindArgs> vSaveBindArgs;

		void clear()
		{
			strLoadProps.clear();
			vLoadBindArgs.clear();
			strSaveProps.clear();
			strSaveParams.clear();
			vSaveBindArgs.clear();
		}
	};*/
}
