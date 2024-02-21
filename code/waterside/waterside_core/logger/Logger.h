#pragma once

#include "LoggerAppender.h"

namespace waterside
{
	/// 类别
	enum LOGGER_CATEGORY
	{
		LOGGER_CATEGORY_DEFAULT,
		LOGGER_CATEGORY_NET,
	};

	/**
	 * 日志
	 */
	template<LOGGER_CATEGORY category = LOGGER_CATEGORY_DEFAULT>
	class TLogger
	{
	public:
		static TLogger<category>& instance()
		{
			static TLogger<category> instance;
			return instance;
		}

		void init(string_view name, 
			LOGGER_LEVEL minLevel =
#ifdef _DEBUG
			LOGGER_LEVEL_TRACE,
#else
			LOGGER_LEVEL_WARN,
#endif
			bool bEnableConsole = true,
			bool bFlushEveryTime = false,
			size_t maxFileSize = 1024 * 1024 * 512)
		{
			mMinLoggerlevel = minLevel;
			mAppender.enableConsole(bEnableConsole);
			mAppender.flushEveryTime(bFlushEveryTime);
			mAppender.setName(name);
			mAppender.setMaxFileSize(maxFileSize);

			mAppender.startThread();
		}

		void stop()
		{
			mAppender.stop();
		}

		void join()
		{
			mAppender.join();
		}

		void setLoggerLevel(LOGGER_LEVEL level) { mMinLoggerlevel = level; }
		LOGGER_LEVEL getLoggerLevel() { return mMinLoggerlevel; }

		template<typename... Args>
		void append(const std::source_location& location, LOGGER_LEVEL level, const string_view fmt, Args&&... args)
		{
			if (!checkLoggerLevel(level))
				return;

			LoggerRecord record(location, std::chrono::system_clock::now(), level);
			try
			{
				record.log = std::vformat(fmt, std::make_format_args(args...));
			}
			catch (const std::exception& e)
			{
				record.log = fmt;
				record.log += " what:";
				record.log += e.what();
			}

			mAppender.append(std::move(record));
		}

	private:
		bool checkLoggerLevel(LOGGER_LEVEL level) const
		{
			return level >= mMinLoggerlevel;
		}

		LOGGER_LEVEL mMinLoggerlevel = LOGGER_LEVEL_TRACE;
		LoggerAppender mAppender;
	};
}

#define LOG_IMPL0(level, ...) waterside::TLogger<>::instance().append(std::source_location::current(), waterside::##level, __VA_ARGS__);
#define LOG_IMPL(category, level, ...) waterside::TLogger<waterside::LOGGER_CATEGORY_##category>::instance().append(std::source_location::current(), waterside::##level, __VA_ARGS__);

#define LOG_TRACE(...) LOG_IMPL0(LOGGER_LEVEL_TRACE, __VA_ARGS__);
#define LOG_DEBUG(...) LOG_IMPL0(LOGGER_LEVEL_DEBUG, __VA_ARGS__);
#define LOG_INFO(...) LOG_IMPL0(LOGGER_LEVEL_INFO, __VA_ARGS__);
#define LOG_WARN(...) LOG_IMPL0(LOGGER_LEVEL_WARN, __VA_ARGS__);
#define LOG_ERROR(...) LOG_IMPL0(LOGGER_LEVEL_ERROR, __VA_ARGS__);
#define LOG_CRITICAL(...) LOG_IMPL0(LOGGER_LEVEL_CRITICAL, __VA_ARGS__);

#define MLOG_TRACE(category, ...) LOG_IMPL(category, LOGGER_LEVEL_TRACE, __VA_ARGS__);
#define MLOG_DEBUG(category, ...) LOG_IMPL(category, LOGGER_LEVEL_DEBUG, __VA_ARGS__);
#define MLOG_INFO(category, ...) LOG_IMPL(category, LOGGER_LEVEL_INFO, __VA_ARGS__);
#define MLOG_WARN(category, ...) LOG_IMPL(category, LOGGER_LEVEL_WARN, __VA_ARGS__);
#define MLOG_ERROR(category, ...) LOG_IMPL(category, LOGGER_LEVEL_ERROR, __VA_ARGS__);
#define MLOG_CRITICAL(category, ...) LOG_IMPL(category, LOGGER_LEVEL_CRITICAL, __VA_ARGS__);
