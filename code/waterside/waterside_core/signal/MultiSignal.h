#pragma once

#include "SignalHandle.h"

namespace waterside
{
	template<typename ...Args>
	class TMultiSignalSlot
	{
	public:
		TMultiSignalSlot()
		{
			mHandle.nextId();
		}

		virtual ~TMultiSignalSlot() = default;

		virtual bool invoke(Args... args) = 0;

		SignalHandle getHandle() const
		{
			return mHandle;
		}
	protected:
		SignalHandle mHandle;
	};

	template<typename ...Args>
	class TStaticMultiSignalSlot : public TMultiSignalSlot<Args...>
	{
	public:
		typedef std::function<void(Args...)> CallbackType;

		TStaticMultiSignalSlot(const CallbackType& callback)
			: mCallback(callback)
		{
		}

		virtual bool invoke(Args... args) override
		{
			mCallback(std::forward<Args>(args)...);
			return true;
		}
	protected:
		CallbackType mCallback;
	};

	template<typename T, typename ...Args>
	class TSharedMultiSignalSlot : public TMultiSignalSlot<Args...>
	{
	public:
		typedef std::function<void(T*, Args...)> CallbackType;

		TSharedMultiSignalSlot(std::shared_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual bool invoke(Args... args) override
		{
			if (mPtr)
			{
				mCallback(mPtr.get(), std::forward<Args>(args)...);
				return true;
			}
			return false;
		}
	protected:
		std::shared_ptr<T> mPtr;
		CallbackType mCallback;
	};

	template<typename T, typename ...Args>
	class TWeakMultiSignalSlot : public TMultiSignalSlot<Args...>
	{
	public:
		typedef std::function<void(T*, Args...)> CallbackType;

		TWeakMultiSignalSlot(std::weak_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual bool invoke(Args... args) override
		{
			if (auto p = mPtr.lock(); p)
			{
				mCallback(p.get(), std::forward<Args>(args)...);
				return true;
			}
			return false;
		}
	protected:
		std::weak_ptr<T> mPtr;
		CallbackType mCallback;
	};


	template<typename Signature> class TMultiSignal;

	template<typename ...Args>
	class TMultiSignal<void(Args...)>
	{
	public:
		SignalHandle connect(const std::function<void(Args...)>& callback)
		{
			auto p = std::make_shared<TStaticMultiSignalSlot<Args...>>(callback);
			mSlots.emplace_back(p);
			return p->getHandle();
		}

		template<typename T>
		SignalHandle connect(std::shared_ptr<T> ptr, const std::function<void(T*, Args...)>& callback)
		{
			auto p = std::make_shared<TSharedMultiSignalSlot<T, Args...>>(ptr, callback);
			mSlots.emplace_back(p);
			return p->getHandle();
		}

		template<typename T>
		SignalHandle connect(std::weak_ptr<T> ptr, const std::function<void(T*, Args...)>& callback)
		{
			auto p = std::make_shared<TWeakMultiSignalSlot<T, Args...>>(ptr, callback);
			mSlots.emplace_back(p);
			return p->getHandle();
		}

		void disconnect(SignalHandle handle)
		{
			for (auto it = mSlots.begin(); it != mSlots.end();)
			{
				if ((*it)->getHandle() == handle)
				{
					it = mSlots.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		bool isValid() const
		{
			return !mSlots.empty();
		}
		operator bool() const
		{
			return isValid();
		}

		void clear()
		{
			mSlots.clear();
		}

		void emit(Args... args)
		{
			for (auto it = mSlots.begin(); it != mSlots.end();)
			{
				if ((*it)->invoke(std::forward<Args>(args)...))
				{
					++it;
				}
				else
				{
					it = mSlots.erase(it);
				}
			}
		}
	private:
		std::vector<std::shared_ptr<TMultiSignalSlot<Args...>>> mSlots;
	};
}
