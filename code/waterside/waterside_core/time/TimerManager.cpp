#include "TimerManager.h"

namespace waterside
{
	void TimerManager::initialize()
	{
		m_startTime = std::chrono::system_clock::now();
	}

	void TimerManager::release()
	{
		for (int i = 0; i < TIME_NEAR; i++)
		{
			while (m_near[i].head)
			{
				auto current = linkClear(&m_near[i]);
				do
				{
					auto temp = current;
					current = current->next;
					destroyNode(temp);
				} while (current);
			}
		}

		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < TIME_LEVEL; i++)
			{
				while (m_level[j][i].head)
				{
					auto current = linkClear(&m_level[j][i]);
					do
					{
						auto temp = current;
						current = current->next;
						destroyNode(temp);
					} while (current);
				}
			}
		}

		m_mapTimer.clear();
	}

	void TimerManager::tick()
	{
		auto now = std::chrono::system_clock::now();
		auto current = interval(std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime).count());

		while (m_current < current)
		{
			++m_current;
			update();
		}
	}

	TimerNode* TimerManager::createNode()
	{
		auto p = m_timerNodeAllocator.allocate(1);
		std::allocator_traits<std::allocator<TimerNode>>{}.construct(m_timerNodeAllocator, p);
		//m_timerNodeAllocator.construct(p);
		return p;
	}

	void TimerManager::destroyNode(TimerNode* p)
	{
		std::allocator_traits<std::allocator<TimerNode>>{}.destroy(m_timerNodeAllocator, p);
		//m_timerNodeAllocator.destroy(p);
		m_timerNodeAllocator.deallocate(p, 1);
	}

	TimerNode* TimerManager::linkClear(TimerLinkList* list)
	{
		auto ret = list->head;
		list->head = nullptr;
		list->tail = nullptr;
		return ret;
	}

	void TimerManager::link(TimerLinkList* list, TimerNode* node)
	{
		node->prev = list->tail;
		if (nullptr == node->prev)
		{
			list->head = node;
		}
		if (list->tail)
		{
			list->tail->next = node;
		}
		list->tail = node;
		node->next = nullptr;
	}

	void TimerManager::linkRemove(TimerLinkList* list, TimerNode* node)
	{
		if (nullptr == node->prev)
		{
			list->head = node->next;
		}
		else
		{
			node->prev->next = node->next;
		}

		if (nullptr == node->next)
		{
			list->tail = node->prev;
		}
		else
		{
			node->next->prev = node->prev;
		}
	}

	void TimerManager::addNode(TimerNode* node)
	{
		uint32_t time = node->expire;

		if ((time | TIME_NEAR_MASK) == (m_current | TIME_NEAR_MASK))
		{
			link(&m_near[time & TIME_NEAR_MASK], node);
		}
		else
		{
			int i;
			uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
			for (i = 0; i < 3; i++)
			{
				if ((time | (mask - 1)) == (m_current | (mask - 1)))
				{
					break;
				}
				mask <<= TIME_LEVEL_SHIFT;
			}

			link(&m_level[i][((time >> (TIME_NEAR_SHIFT + i * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
		}
	}

	void TimerManager::moveList(int level, int idx)
	{
		auto current = linkClear(&m_level[level][idx]);
		while (current)
		{
			auto temp = current->next;
			addNode(current);
			current = temp;
		}
	}

	void TimerManager::shift()
	{
		int mask = TIME_NEAR;
		uint32_t ct = m_current;
		if (ct == 0)
		{
			moveList(3, 0);
		}
		else
		{
			uint32_t time = ct >> TIME_NEAR_SHIFT;
			int i = 0;

			while ((ct & (mask - 1)) == 0)
			{
				int idx = time & TIME_LEVEL_MASK;
				if (idx != 0)
				{
					moveList(i, idx);
					break;
				}
				mask <<= TIME_LEVEL_SHIFT;
				time >>= TIME_LEVEL_SHIFT;
				++i;
			}
		}
	}

	void TimerManager::execute()
	{
		int idx = m_current & TIME_NEAR_MASK;

		while (m_near[idx].head)
		{
			auto current = linkClear(&m_near[idx]);
			do
			{
				current->callback();
				m_mapTimer.erase(current->handle);

				auto temp = current;
				current = current->next;
				destroyNode(temp);
			} while (current);
		}
	}

	void TimerManager::update()
	{
		execute();

		shift();

		execute();
	}

	uint32_t TimerManager::createTimer(uint32_t expire, const std::function<void()>& callback)
	{
		if (++m_handle == 0)
		{
			++m_handle;
		}
		auto node = createNode();
		node->prev = nullptr;
		node->next = nullptr;
		node->handle = m_handle;
		node->expire = expire;
		node->callback = callback;
		m_mapTimer[m_handle] = node;
		addNode(node);
		return m_handle;
	}

	void TimerManager::del(uint32_t handle)
	{
		if (m_handle != 0)
		{
			auto it = m_mapTimer.find(handle);
			if (it != m_mapTimer.end())
			{
				auto node = it->second;

				uint32_t time = node->expire;

				if ((time | TIME_NEAR_MASK) == (m_current | TIME_NEAR_MASK))
				{
					linkRemove(&m_near[time & TIME_NEAR_MASK], node);
				}
				else
				{
					int i;
					uint32_t mask = TIME_NEAR << TIME_LEVEL_SHIFT;
					for (i = 0; i < 3; i++)
					{
						if ((time | (mask - 1)) == (m_current | (mask - 1)))
						{
							break;
						}
						mask <<= TIME_LEVEL_SHIFT;
					}

					linkRemove(&m_level[i][((time >> (TIME_NEAR_SHIFT + i * TIME_LEVEL_SHIFT)) & TIME_LEVEL_MASK)], node);
				}

				m_mapTimer.erase(it);

				destroyNode(node);
			}
		}
	}
}
