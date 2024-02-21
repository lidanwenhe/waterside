#include "LoggerRecord.h"
#include <thread>

namespace waterside
{
	LoggerRecord::LoggerRecord(const std::source_location& location, const std::chrono::system_clock::time_point& tp, LOGGER_LEVEL lv)
		: mLocation(location)
		, mTp(tp)
		, mLevel(lv)
		, mThreadId(getThreadId())
	{
	}

	size_t LoggerRecord::getThreadId()
	{
        thread_local static size_t tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
		return tid;
	}

	string LoggerRecord::recordString() const
	{
		return std::format("{} {} [{}] ({}:{}<{}>) {}\n",
			mTp,
			getLoggerLevelName(mLevel),
			mThreadId,
			mLocation.file_name(),
			mLocation.line(),
			mLocation.function_name(),
			log);
	}
}
