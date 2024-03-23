#pragma once

#include "prerequisites.h"

namespace waterside
{
	// 参考Twitter的分布式自增ID算法
	class SnowflakeId final : public TLazySingleton<SnowflakeId>
	{
	public:
		SnowflakeId();

		bool initialize(int32_t workerId = 0, int32_t workerIdBits = 10, int32_t sequenceBits = 12);

		unique_id nextId();

	protected:
		int64_t getTimestamp();
		int64_t getNextTimestamp(int64_t lastTimestamp);

	private:
		// 工作机器ID
		int64_t mWorkerId;
		// 生成序列的掩码
		int32_t mSequenceMask;
		// 毫秒内序列
		int32_t mSequence;
		// 时间戳偏移
		int32_t mTimestampLeftShift;
		// 时间戳的掩码
		int64_t mTimestampMask;
		// 上次生成ID的时间截
		int64_t mLastTimestamp;
		// 从某个时间开始计算
		int64_t mTwepoch;

		std::mutex mMutex;
	};
}
