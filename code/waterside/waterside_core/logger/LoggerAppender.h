#pragma once

#include "LoggerRecord.h"
#include "tbb/concurrent_queue.h"

namespace waterside
{
	/**
	 * 日志附加器
	 */
	class LoggerAppender
	{
	public:
		LoggerAppender() = default;
		~LoggerAppender() = default;

		void enableConsole(bool b)
		{
			mEnableConsole = b;
		}
		void flushEveryTime(bool b)
		{
			mFlushEveryTime = b;
		}
		void setName(string_view name)
		{
			mName = name;
		}
		void setMaxFileSize(size_t size)
		{
			mMaxFileSize = size;
		}

		void startThread();

		void stop();

		void join();

		void append(LoggerRecord&& record);

	private:
		void writeRecord(const LoggerRecord& record);
#ifdef _WIN32
		enum class color_type : int
		{
			none = -1,
			black = 0,
			blue,
			green,
			cyan,
			red,
			magenta,
			yellow,
			white,
			black_bright,
			blue_bright,
			green_bright,
			cyan_bright,
			red_bright,
			magenta_bright,
			yellow_bright,
			white_bright
		};

		void windows_set_color(color_type fg, color_type bg);
#endif
		void add_color(LOGGER_LEVEL lv);
		void clean_color(LOGGER_LEVEL lv);
		
		void openLogFile(const LoggerRecord& record);

	private:
		bool mEnableConsole = true;
		bool mFlushEveryTime = false;
		string mName;
		size_t mMaxFileSize = 1024 * 1024;
		std::ofstream mFile;
		string mFilename;
		size_t mCurrentFileSize = 0;
		std::chrono::year_month_day mYmd;

		tbb::concurrent_queue<LoggerRecord> mRecords;
		std::thread mThread;
		std::mutex mMutex;
		std::condition_variable mCnd;
		std::atomic<bool> mStop = false;
	};
}
