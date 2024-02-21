#include "SnowflakeId.h"
#include "Logger.h"

namespace waterside
{
	SnowflakeId::SnowflakeId()
		: mbInit(false)
		, mWorkerId(0)
		, mSequenceMask(0)
		, mSequence(0)
		, mTimestampLeftShift(0)
		, mTimestampMask(0)
		, mLastTimestamp(0)
		, mTwepoch(1704038400000LL) // 2024/1/1
	{
	}

	bool SnowflakeId::initialize(int32_t workerId, int32_t workerIdBits, int32_t sequenceBits)
	{
		if (workerId <= 0 || workerIdBits <= 0 || sequenceBits <= 0 || workerIdBits + sequenceBits != 22)
		{
			LOG_ERROR("init error, workerId:{}, workerIdBits:{}, sequenceBits:{}", workerId, workerIdBits, sequenceBits);
			return false;
		}

		mWorkerId = workerId;
		// 支持的最大工作机器id
		const int64_t maxWorkerId = (1LL << workerIdBits) - 1;
		if (mWorkerId > maxWorkerId)
		{
			LOG_ERROR("worker Id {} can't be greater than {} or less than 0", mWorkerId, maxWorkerId);
			return false;
		}

		// 工作机器ID偏移
		const int64_t workerIdShift = sequenceBits;
		// 时间戳偏移
		mTimestampLeftShift = sequenceBits + workerIdBits;
		// 生成序列的掩码
		mSequenceMask = (1L << sequenceBits) - 1;
		// 时间戳的掩码
		mTimestampMask = (1LL << (63 - mTimestampLeftShift)) - 1;

		mWorkerId <<= workerIdShift;

		mbInit = true;
		return true;
	}

	unique_id SnowflakeId::nextId()
	{
		assert(mbInit);

		std::lock_guard<std::mutex> lock(mMutex);

		int64_t timestamp = getTimestamp();

		if (timestamp < mLastTimestamp)
		{
			// 如果当前时间小于上一次ID生成的时间戳，说明系统时钟回退过这个时候应当抛出异常
			LOG_ERROR("Clock moved backwards.  Refusing to generate id for {} milliseconds", mLastTimestamp - timestamp);
			assert(false);
			return 0;
		}

		// 如果是同一时间生成的，则进行毫秒内序列
		if (mLastTimestamp == timestamp)
		{
			mSequence = (mSequence + 1) & mSequenceMask;
			// 毫秒内序列溢出
			if (mSequence == 0)
			{
				//阻塞到下一个毫秒,获得新的时间戳
				timestamp = getNextTimestamp(mLastTimestamp);
			}
		}
		//时间戳改变，毫秒内序列重置
		else
		{
			mSequence = 0;
		}

		//上次生成ID的时间截
		mLastTimestamp = timestamp;

		//移位并通过或运算拼到一起组成64位的ID
		return (((timestamp - mTwepoch) & mTimestampMask) << mTimestampLeftShift)
			| mWorkerId
			| mSequence;
	}

	int64_t SnowflakeId::getTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto durationInMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
		return durationInMs.count();
	}

	int64_t SnowflakeId::getNextTimestamp(int64_t lastTimestamp)
	{
		int64_t timestamp = getTimestamp();
		while (timestamp <= lastTimestamp)
		{
			timestamp = getTimestamp();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		return timestamp;
	}
}
