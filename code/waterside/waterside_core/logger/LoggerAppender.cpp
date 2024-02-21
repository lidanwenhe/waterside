#include "LoggerAppender.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
#endif

namespace waterside
{
	void LoggerAppender::startThread()
	{
        mThread = std::thread([this] {
            while (!mStop)
            {
                LoggerRecord record;
                if (mRecords.try_pop(record))
                {
                    writeRecord(record);
                }

                if (mRecords.empty())
                {
                    std::unique_lock lock(mMutex);
                    mCnd.wait(lock, [&]() {
                        return !mRecords.empty() || mStop;
                        });
                }
            }

            LoggerRecord record;
            while (mRecords.try_pop(record))
            {
                writeRecord(record);
            }
            });
	}

    void LoggerAppender::stop()
    {
        std::unique_lock lock(mMutex);

        if (!mThread.joinable())
            return;

        if (mStop)
            return;

        mStop = true;
        mCnd.notify_one();
    }

    void LoggerAppender::join()
    {
        stop();
        if (mThread.joinable())
            mThread.join();
    }

	void LoggerAppender::append(LoggerRecord&& record)
	{
		mRecords.push(record);

        std::unique_lock lock(mMutex);
        mCnd.notify_one();
	}

#ifdef _WIN32
    void LoggerAppender::windows_set_color(color_type fg, color_type bg)
    {
        auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (handle != nullptr) {
            CONSOLE_SCREEN_BUFFER_INFO info{};
            auto status = GetConsoleScreenBufferInfo(handle, &info);
            if (status) {
                WORD color = info.wAttributes;
                if (fg != color_type::none) {
                    color = (color & 0xFFF0) | int(fg);
                }
                if (bg != color_type::none) {
                    color = (color & 0xFF0F) | int(bg) << 4;
                }
                SetConsoleTextAttribute(handle, color);
            }
        }
    }
#endif

    void LoggerAppender::add_color(LOGGER_LEVEL lv)
    {
#if defined(_WIN32)
        if (lv == LOGGER_LEVEL_WARN)
            windows_set_color(color_type::black, color_type::yellow);
        if (lv == LOGGER_LEVEL_ERROR)
            windows_set_color(color_type::black, color_type::red);
        if (lv == LOGGER_LEVEL_CRITICAL)
            windows_set_color(color_type::white_bright, color_type::red);
#elif __APPLE__
#else
        if (lv == LOGGER_LEVEL_WARN)
            std::cout << "\x1B[93m";
        if (lv == LOGGER_LEVEL_ERROR)
            std::cout << "\x1B[91m";
        if (lv == LOGGER_LEVEL_CRITICAL)
            std::cout << "\x1B[97m\x1B[41m";
#endif
    }
    void LoggerAppender::clean_color(LOGGER_LEVEL lv)
    {
#if defined(_WIN32)
        if (lv >= LOGGER_LEVEL_WARN)
            windows_set_color(color_type::white, color_type::black);
#elif __APPLE__
#else
        if (lv >= LOGGER_LEVEL_WARN)
            std::cout << "\x1B[0m\x1B[0K";
#endif
    }

    void LoggerAppender::writeRecord(const LoggerRecord& record)
    {
        auto str = record.recordString();

        if (mEnableConsole)
        {
            add_color(record.getLoggerLevel());
            std::cerr << str;
            clean_color(record.getLoggerLevel());
        }

        openLogFile(record);

        if (mFile.write(str.data(), str.size()))
        {
            if (mFlushEveryTime || record.getLoggerLevel() == LOGGER_LEVEL_CRITICAL)
            {
                mFile.flush();
            }
            mCurrentFileSize += str.size();
        }
    }

    void LoggerAppender::openLogFile(const LoggerRecord& record)
    {
        const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(record.getTimePoint()) };
        if (mFile.is_open() && mCurrentFileSize < mMaxFileSize && mYmd == ymd)
            return;

        mFile.close();
        if (mFilename.empty())
        {
            mFilename = std::format("../../log/{:%F} {}.log", record.getTimePoint(), mName);
            mYmd = ymd;
        }
        else if (mYmd != ymd)
        {
            mYmd = ymd;
        }
        else if (mCurrentFileSize >= mMaxFileSize)
        {
            int number = 1;
            auto bakname = std::format("{}.{}", mFilename, number);
            while (std::filesystem::exists(bakname))
            {
                bakname = std::format("{}.{}", mFilename, ++number);
            }

            std::filesystem::rename(mFilename, bakname);
        }

        mFile.open(mFilename, std::ios::binary | std::ios::out | std::ios::app);
        mCurrentFileSize = std::filesystem::file_size(mFilename);
    }
}