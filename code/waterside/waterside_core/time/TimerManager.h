#pragma once

#include "prerequisites.h"

#define TIME_NEAR_SHIFT 8
#define TIME_NEAR (1 << TIME_NEAR_SHIFT)
#define TIME_LEVEL_SHIFT 6
#define TIME_LEVEL (1 << TIME_LEVEL_SHIFT)
#define TIME_NEAR_MASK (TIME_NEAR-1)
#define TIME_LEVEL_MASK (TIME_LEVEL-1)

namespace waterside
{
	struct TimerNode final
	{
		TimerNode* prev = nullptr;
		TimerNode* next = nullptr;
		uint32_t handle = 0;
		uint32_t expire = 0;
		std::function<void()> callback;
	};

	struct TimerLinkList final
	{
		TimerNode* head = nullptr;
		TimerNode* tail = nullptr;
	};

	class TimerManager final : public TLazySingleton<TimerManager>
	{
	public:
		void initialize();

		void release();

		void tick();

		template <class Rep, class Period>
		uint32_t delay(const std::chrono::duration<Rep, Period>& delayTime, const std::function<void()>& callback)
		{
			auto now = std::chrono::system_clock::now();
			auto expire = interval(std::chrono::duration_cast<std::chrono::milliseconds>(now + delayTime - m_startTime).count());
			return createTimer(expire, callback);
		}

		uint32_t at(const std::chrono::system_clock::time_point& atTime, const std::function<void()>& callback)
		{
			auto expire = interval(std::chrono::duration_cast<std::chrono::milliseconds>(atTime - m_startTime).count());
			return createTimer(expire, callback);
		}

		void del(uint32_t handle);

	protected:
		uint32_t interval(long long duration)
		{
			return (uint32_t)((duration + 5) / 10);
		}

		TimerNode* createNode();
		void destroyNode(TimerNode* p);

		TimerNode* linkClear(TimerLinkList* list);
		void link(TimerLinkList* list, TimerNode* node);
		void linkRemove(TimerLinkList* list, TimerNode* node);

		void addNode(TimerNode* node);
		void moveList(int level, int idx);
		void shift();
		void execute();
		void update();

		uint32_t createTimer(uint32_t expire, const std::function<void()>& callback);

	private:
		std::allocator<TimerNode> m_timerNodeAllocator;
		std::unordered_map<uint32_t, TimerNode*> m_mapTimer;

		TimerLinkList m_near[TIME_NEAR];
		TimerLinkList m_level[4][TIME_LEVEL];
		std::chrono::system_clock::time_point m_startTime;
		uint32_t m_current = 0;
		uint32_t m_handle = 0;
	};
}
