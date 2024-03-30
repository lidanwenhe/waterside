#pragma once

#include "IoContextPool.h"
#include "Logger.h"
#include <mysql.h>
#include <errmsg.h>
#include <mysqld_error.h>

namespace waterside
{
	struct MysqlLibrary : private boost::noncopyable
	{
		MysqlLibrary()
		{
			mysql_library_init(0, nullptr, nullptr);
		}
		~MysqlLibrary()
		{
			mysql_library_end();
		}
	};

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
}
