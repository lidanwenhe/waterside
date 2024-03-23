#pragma once

#include "MysqlPrerequisites.h"

namespace waterside
{
	class MysqlException
	{
	public:
		template <typename... T>
		MysqlException(const std::source_location& location, const std::string_view fmt, T&&... args)
			: mErrno(0)
		{
			try
			{
				mWhat = std::vformat(fmt, std::make_format_args(args...));
				std::format_to(std::back_inserter(mWhat), " ({}:{}[{}])", location.file_name(), location.line(), location.function_name());
			}
			catch (const std::exception& e)
			{
				mWhat = fmt;
			}
		}

		template <typename... T>
		MysqlException(const std::source_location& location, unsigned int _errno,
			const char* _error, const char* _state, const std::string_view fmt, T&&... args)
			: mErrno(0)
		{
			try
			{
				mWhat = std::vformat(fmt, std::make_format_args(args...));
				std::format_to(std::back_inserter(mWhat), "[errno:{}, error:{}, state:{}] ({}:{}<{}>)", _errno, _error, _state, location.file_name(), location.line(), location.function_name());
			}
			catch (const std::exception& e)
			{
				mWhat = fmt;
			}
		}

		std::string_view what() const
		{
			return mWhat;
		}

		unsigned int geterrno() const
		{
			return mErrno;
		}

	private:
		string mWhat;
		unsigned int mErrno;
	};
}

#define MYSQL_DB_THROW(...) throw waterside::MysqlException(std::source_location::current(), __VA_ARGS__)
