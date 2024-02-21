#pragma once

#include "prerequisites.h"

namespace waterside
{
	/// 日志输出等级
	enum LOGGER_LEVEL
	{
		/// 无
		LOGGER_LEVEL_NONE,
		/// 追踪
		LOGGER_LEVEL_TRACE,
		/// 调试
		LOGGER_LEVEL_DEBUG,
		/// 信息
		LOGGER_LEVEL_INFO,
		/// 警告
		LOGGER_LEVEL_WARN,
		LOGGER_LEVEL_WARNING = LOGGER_LEVEL_WARN,
		/// 错误
		LOGGER_LEVEL_ERROR,
		/// 关键
		LOGGER_LEVEL_CRITICAL,
		LOGGER_LEVEL_FATAL = LOGGER_LEVEL_CRITICAL,
	};

	inline string_view getLoggerLevelName(LOGGER_LEVEL lv)
	{
		switch (lv)
		{
		case LOGGER_LEVEL_TRACE:
			return "TRACE   ";
		case LOGGER_LEVEL_DEBUG:
			return "DEBUG   ";
		case LOGGER_LEVEL_INFO:
			return "INFO    ";
		case LOGGER_LEVEL_WARN:
			return "WARNING ";
		case LOGGER_LEVEL_ERROR:
			return "ERROR   ";
		case LOGGER_LEVEL_CRITICAL:
			return "CRITICAL";
		default:
			return "NONE    ";
		}
	}

	/// 日志记录
	class LoggerRecord
	{
	public:
		LoggerRecord() = default;

		LoggerRecord(const std::source_location& location, const std::chrono::system_clock::time_point& tp, LOGGER_LEVEL lv);

		const std::chrono::system_clock::time_point& getTimePoint() const
		{
			return mTp;
		}

		LOGGER_LEVEL getLoggerLevel() const
		{
			return mLevel;
		}

		string recordString() const;

		string log;

	private:
		size_t getThreadId();

		std::source_location mLocation;
		std::chrono::system_clock::time_point mTp;
		LOGGER_LEVEL mLevel;
		size_t mThreadId = 0;
	};
}
