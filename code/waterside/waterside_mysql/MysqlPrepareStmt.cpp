#include "MysqlPrepareStmt.h"
#include "MysqlException.h"

namespace waterside
{
	MysqlPrepareStmt::MysqlPrepareStmt()
		: mpStmt(nullptr)
		, mIndex(0)
		, mbProcess(false)
	{
	}

	MysqlPrepareStmt::~MysqlPrepareStmt()
	{
		close();
	}

	void MysqlPrepareStmt::initialize(int32_t index, std::string_view sql, const vector<MysqlStmtBindArgs>& vParams, const vector<MysqlStmtBindArgs>& vResults)
	{
		mIndex = index;
		if (sql.empty())
		{
			MYSQL_DB_THROW("sql is empty,index={}"sv, mIndex);
		}
		mSql = sql;
		if (!vResults.empty() && sql.size() >= 4 &&
			(sql[0] == 'c' || sql[0] == 'C') &&
			(sql[1] == 'a' || sql[1] == 'A') &&
			(sql[2] == 'l' || sql[2] == 'L') &&
			(sql[3] == 'l' || sql[3] == 'L'))
		{// 有返回的存储过程
			mbProcess = true;
		}

		mBindParams.resize(vParams.size());
		mBindResults.resize(vResults.size());


		mBuffer.resize(calcBufferSize(vParams, vResults));
		auto p = mBuffer.data();
		for (auto& item : mBindParams)
		{
			item.buffer = p + (size_t)item.buffer;
			if (item.is_null != nullptr)
			{
				item.is_null = reinterpret_cast<bool*>(p + (size_t)item.is_null);
			}
			if (item.length != nullptr)
			{
				item.length = reinterpret_cast<unsigned long*>(p + (size_t)item.length);
			}
		}
		for (auto& item : mBindResults)
		{
			item.buffer = p + (size_t)item.buffer;
			if (item.is_null != nullptr)
			{
				item.is_null = reinterpret_cast<bool*>(p + (size_t)item.is_null);
			}
			if (item.length != nullptr)
			{
				item.length = reinterpret_cast<unsigned long*>(p + (size_t)item.length);
			}
		}
	}

	size_t MysqlPrepareStmt::calcBufferSize(const vector<MysqlStmtBindArgs>& vParams, const vector<MysqlStmtBindArgs>& vResults)
	{
		size_t offset = 0;
		size_t nAlignment = 1;
		size_t n = vParams.size();
		for (size_t i = 0; i < n; i++)
		{
			auto& arg = vParams[i];
			auto& item = mBindParams[i];
			callBindItem(offset, nAlignment, arg, item);
		}
		n = vResults.size();
		for (size_t i = 0; i < n; i++)
		{
			auto& arg = vResults[i];
			auto& item = mBindResults[i];
			callBindItem(offset, nAlignment, arg, item);
		}
		return offset;
	}

	void MysqlPrepareStmt::callBindItem(size_t& offset, size_t& nAlignment, const MysqlStmtBindArgs& arg, MYSQL_BIND& item)
	{
		memset(&item, 0, sizeof(MYSQL_BIND));

		switch (arg.type)
		{
		case MYSQL_STMT_TYPE::BYTE:
			item.buffer_type = MYSQL_TYPE_TINY;
			item.is_unsigned = false;
			break;
		case MYSQL_STMT_TYPE::UBYTE:
			item.buffer_type = MYSQL_TYPE_TINY;
			item.is_unsigned = true;
			break;
		case MYSQL_STMT_TYPE::SHORT:
			item.buffer_type = MYSQL_TYPE_SHORT;
			item.is_unsigned = false;
			break;
		case MYSQL_STMT_TYPE::USHORT:
			item.buffer_type = MYSQL_TYPE_SHORT;
			item.is_unsigned = true;
			break;
		case MYSQL_STMT_TYPE::INT:
			item.buffer_type = MYSQL_TYPE_LONG;
			item.is_unsigned = false;
			break;
		case MYSQL_STMT_TYPE::UINT:
			item.buffer_type = MYSQL_TYPE_LONG;
			item.is_unsigned = true;
			break;
		case MYSQL_STMT_TYPE::LONG:
			item.buffer_type = MYSQL_TYPE_LONGLONG;
			item.is_unsigned = false;
			break;
		case MYSQL_STMT_TYPE::ULONG:
			item.buffer_type = MYSQL_TYPE_LONGLONG;
			item.is_unsigned = true;
			break;
		case MYSQL_STMT_TYPE::FLOAT:
			item.buffer_type = MYSQL_TYPE_FLOAT;
			break;
		case MYSQL_STMT_TYPE::DOUBLE:
			item.buffer_type = MYSQL_TYPE_DOUBLE;
			break;
		case MYSQL_STMT_TYPE::STRING:
			item.buffer_type = MYSQL_TYPE_STRING;
			break;
		case MYSQL_STMT_TYPE::VARSTRING:
			item.buffer_type = MYSQL_TYPE_VAR_STRING;
			break;
		case MYSQL_STMT_TYPE::TINYBLOB:
			// 255 byte
			item.buffer_type = MYSQL_TYPE_TINY_BLOB;
			break;
		case MYSQL_STMT_TYPE::BLOB:
			// (2^16-1) byte
			item.buffer_type = MYSQL_TYPE_BLOB;
			break;
		case MYSQL_STMT_TYPE::MEDIUMBLOB:
			// (2^24-1) byte
			item.buffer_type = MYSQL_TYPE_MEDIUM_BLOB;
			break;
		default:
			assert(false);
			MYSQL_DB_THROW("index={},sql={}, arg type error"sv, mIndex, mSql);
			break;
		}

		switch (arg.type)
		{
		case MYSQL_STMT_TYPE::BYTE:
		case MYSQL_STMT_TYPE::UBYTE:
			nAlignment = 1;
			item.buffer = (void*)offset;
			++offset;
			break;
		case MYSQL_STMT_TYPE::SHORT:
		case MYSQL_STMT_TYPE::USHORT:
			if (nAlignment < 2)
			{
				offset = (offset + 1) / 2 * 2;
			}
			nAlignment = 2;

			item.buffer = (void*)offset;
			offset += 2;
			break;
		case MYSQL_STMT_TYPE::INT:
		case MYSQL_STMT_TYPE::UINT:
		case MYSQL_STMT_TYPE::FLOAT:
			if (nAlignment < 4)
			{
				offset = (offset + 3) / 4 * 4;
			}
			nAlignment = 4;

			item.buffer = (void*)offset;
			offset += 4;
			break;
		case MYSQL_STMT_TYPE::LONG:
		case MYSQL_STMT_TYPE::ULONG:
		case MYSQL_STMT_TYPE::DOUBLE:
			if (nAlignment < 8)
			{
				offset = (offset + 7) / 8 * 8;
			}
			nAlignment = 8;

			item.buffer = (void*)offset;
			offset += 8;
			break;
		case MYSQL_STMT_TYPE::STRING:
		case MYSQL_STMT_TYPE::VARSTRING:
		case MYSQL_STMT_TYPE::TINYBLOB:
		{
			auto nSize = (arg.size > 0 ? arg.size : 255);

			nAlignment = 1;
			item.buffer = (void*)offset;
			offset += nSize;
			item.buffer_length = nSize;
		}
		break;
		case MYSQL_STMT_TYPE::BLOB:
		case MYSQL_STMT_TYPE::MEDIUMBLOB:
		{
			auto nSize = (arg.size > 0 ? arg.size : 1024);

			nAlignment = 1;
			item.buffer = (void*)offset;
			offset += nSize;
			item.buffer_length = nSize;
		}
		default:
			break;
		}

		switch (arg.type)
		{
		case MYSQL_STMT_TYPE::STRING:
		case MYSQL_STMT_TYPE::VARSTRING:
		case MYSQL_STMT_TYPE::TINYBLOB:
		case MYSQL_STMT_TYPE::BLOB:
		case MYSQL_STMT_TYPE::MEDIUMBLOB:
		{
			if (arg.bNull)
			{
				nAlignment = 1;
				item.is_null = (bool*)offset;
				++offset;
			}

			size_t a = alignof(unsigned long);
			if (nAlignment < a)
			{
				offset = (offset + a - 1) / a * a;
			}
			nAlignment = a;

			item.length = (unsigned long*)offset;
			offset += a;
		}
		break;
		default:
			break;
		}
	}

	void MysqlPrepareStmt::boundParams()
	{
		if (mBindParams.empty())
		{
			return;
		}

		if (params.size() != mBindParams.size())
		{
			MYSQL_DB_THROW("mysql bound params size error:{}, need:{}"sv, params.size(), mBindParams.size());
		}

		size_t n = params.size();
		for (size_t i = 0; i < n; i++)
		{
			auto& arg = params[i];
			auto& item = mBindParams[i];
			switch (item.buffer_type)
			{
			case MYSQL_TYPE_TINY:
				if (item.is_unsigned)
				{
					*reinterpret_cast<uint8_t*>(item.buffer) = (uint8_t)std::get<int32_t>(arg);
				}
				else
				{
					*reinterpret_cast<int8_t*>(item.buffer) = (int8_t)std::get<int32_t>(arg);
				}
				break;
			case MYSQL_TYPE_SHORT:
				if (item.is_unsigned)
				{
					*reinterpret_cast<uint16_t*>(item.buffer) = (uint16_t)std::get<int32_t>(arg);
				}
				else
				{
					*reinterpret_cast<int16_t*>(item.buffer) = (int16_t)std::get<int32_t>(arg);
				}
				break;
			case MYSQL_TYPE_LONG:
				if (item.is_unsigned)
				{
					*reinterpret_cast<uint32_t*>(item.buffer) = (uint32_t)std::get<int32_t>(arg);
				}
				else
				{
					*reinterpret_cast<int32_t*>(item.buffer) = std::get<int32_t>(arg);
				}
				break;
			case MYSQL_TYPE_LONGLONG:
				if (item.is_unsigned)
				{
					*reinterpret_cast<uint64_t*>(item.buffer) = (uint64_t)std::get<int64_t>(arg);
				}
				else
				{
					*reinterpret_cast<int64_t*>(item.buffer) = std::get<int64_t>(arg);
				}
				break;
			case MYSQL_TYPE_FLOAT:
				*reinterpret_cast<float*>(item.buffer) = std::get<float>(arg);
				break;
			case MYSQL_TYPE_DOUBLE:
				*reinterpret_cast<double*>(item.buffer) = std::get<double>(arg);
				break;
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
			{
				auto value = std::get<std::string_view>(arg);
				if (item.is_null)
				{
					*item.is_null = value.empty();
				}
				if (value.empty())
				{
					*item.length = 0;
				}
				else
				{
					auto len = std::min(item.buffer_length, (unsigned long)value.size());
					memcpy(item.buffer, value.data(), len);
					*item.length = len;
				}
			}
			break;
			default:
				break;
			}
		}
	}

	void MysqlPrepareStmt::storeResults()
	{
		results.clear();
		for (const auto& item : mBindResults)
		{
			switch (item.buffer_type)
			{
			case MYSQL_TYPE_TINY:
				if (item.is_unsigned)
				{
					results.emplace_back((int32_t) * reinterpret_cast<uint8_t*>(item.buffer));
				}
				else
				{
					results.emplace_back((int32_t) * reinterpret_cast<int8_t*>(item.buffer));
				}
				break;
			case MYSQL_TYPE_SHORT:
				if (item.is_unsigned)
				{
					results.emplace_back((int32_t) * reinterpret_cast<uint16_t*>(item.buffer));
				}
				else
				{
					results.emplace_back((int32_t) * reinterpret_cast<int16_t*>(item.buffer));
				}
				break;
			case MYSQL_TYPE_LONG:
				if (item.is_unsigned)
				{
					results.emplace_back((int32_t) * reinterpret_cast<uint32_t*>(item.buffer));
				}
				else
				{
					results.emplace_back(*reinterpret_cast<int32_t*>(item.buffer));
				}
				break;
			case MYSQL_TYPE_LONGLONG:
				if (item.is_unsigned)
				{
					results.emplace_back((int64_t) * reinterpret_cast<uint64_t*>(item.buffer));
				}
				else
				{
					results.emplace_back(*reinterpret_cast<int64_t*>(item.buffer));
				}
				break;
			case MYSQL_TYPE_FLOAT:
				results.emplace_back(*reinterpret_cast<float*>(item.buffer));
				break;
			case MYSQL_TYPE_DOUBLE:
				results.emplace_back(*reinterpret_cast<double*>(item.buffer));
				break;
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_TINY_BLOB:
			case MYSQL_TYPE_BLOB:
			case MYSQL_TYPE_MEDIUM_BLOB:
				if (item.is_null || *item.length == 0)
				{
					results.emplace_back(std::string_view{});
				}
				else
				{
					results.emplace_back(std::string_view{ (const char*)item.buffer, *item.length });
				}
				break;
			default:
				break;
			}
		}
	}

	void MysqlPrepareStmt::connect(MYSQL* pMysql)
	{
		close();

		mpStmt = mysql_stmt_init(pMysql);
		if (!mpStmt)
		{
			MYSQL_DB_THROW(mysql_errno(pMysql), mysql_error(pMysql), mysql_sqlstate(pMysql), "mysql_stmt_init fail,index={},sql={}"sv, mIndex, mSql);
		}

		if (mysql_stmt_prepare(mpStmt, mSql.c_str(), (unsigned long)mSql.size()))
		{
			uint32_t uerrno = mysql_errno(pMysql);
			string strerror = mysql_error(pMysql);
			string strstate = mysql_sqlstate(pMysql);

			mysql_stmt_close(mpStmt);
			mpStmt = nullptr;

			MYSQL_DB_THROW(uerrno, strerror.c_str(), strstate.c_str(), "mysql_stmt_prepare fail,index={},sql={}"sv, mIndex, mSql);
		}

		bindComplete();
	}

	void MysqlPrepareStmt::close()
	{
		if (mpStmt)
		{
			mysql_stmt_close(mpStmt);
			mpStmt = nullptr;
		}
	}

	void MysqlPrepareStmt::bindComplete()
	{
		bindParam();

		if (!(mBindResults.empty() || mbProcess))
		{
			bindResult();
		}
	}

	void MysqlPrepareStmt::bindParam()
	{
		if (0 != mysql_stmt_bind_param(mpStmt, mBindParams.data()))
		{
			MYSQL_DB_THROW(mysql_stmt_errno(mpStmt), mysql_stmt_error(mpStmt), mysql_stmt_sqlstate(mpStmt), "mysql_stmt_bind_param fail,index={},sql={}"sv, mIndex, mSql);
		}

		unsigned long size = mysql_stmt_param_count(mpStmt);
		if (size != mBindParams.size())
		{
			MYSQL_DB_THROW("index={},sql={}, bind param size({}) != max({})"sv, mIndex, mSql, size, mBindParams.size());
		}
	}

	void MysqlPrepareStmt::bindResult()
	{
		if (0 != mysql_stmt_bind_result(mpStmt, mBindResults.data()))
		{
			MYSQL_DB_THROW(mysql_stmt_errno(mpStmt), mysql_stmt_error(mpStmt), mysql_stmt_sqlstate(mpStmt), "mysql_stmt_bind_result fail,index={},sql={}"sv, mIndex, mSql);
		}
	}

	void MysqlPrepareStmt::execute()
	{
		boundParams();

		if (0 != mysql_stmt_execute(mpStmt))
		{
			MYSQL_DB_THROW(mysql_stmt_errno(mpStmt), mysql_stmt_error(mpStmt), mysql_stmt_sqlstate(mpStmt), "mysql_stmt_execute fail,index={},sql={}"sv, mIndex, mSql);
		}

		if (mBindResults.empty())
		{
			return;
		}

		if (0 != mysql_stmt_store_result(mpStmt))
		{
			MYSQL_DB_THROW(mysql_stmt_errno(mpStmt), mysql_stmt_error(mpStmt), mysql_stmt_sqlstate(mpStmt), "mysql_stmt_store_result fail,index={},sql={}"sv, mIndex, mSql);
		}

		if (mbProcess)
		{
			bindResult();
		}
	}

	bool MysqlPrepareStmt::fetch()
	{
		int ret = mysql_stmt_fetch(mpStmt);
		if (1 == ret)
		{
			MYSQL_DB_THROW(mysql_stmt_errno(mpStmt), mysql_stmt_error(mpStmt), mysql_stmt_sqlstate(mpStmt), "mysql_stmt_fetch fail,index={},sql={}"sv, mIndex, mSql);
		}

		if (MYSQL_NO_DATA != ret)
		{
			storeResults();
			return true;
		}
		return false;
	}

	void MysqlPrepareStmt::clear()
	{
		params.clear();
		results.clear();

		if (mBindResults.empty())
		{
			return;
		}

		//取没有取完的
		while (0 == mysql_stmt_fetch(mpStmt));

		if (!mbProcess)
		{
			return;
		}

		do
		{
			MYSQL_RES* result = mysql_store_result(mpStmt->mysql);
			mysql_free_result(result);
		} while (!mysql_next_result(mpStmt->mysql));
	}

	uint64_t MysqlPrepareStmt::getInsertId()
	{
		return (uint64_t)mysql_stmt_insert_id(mpStmt);
	}

	uint64_t MysqlPrepareStmt::getAffectedRows()
	{
		return (uint64_t)mysql_stmt_affected_rows(mpStmt);
	}
}
