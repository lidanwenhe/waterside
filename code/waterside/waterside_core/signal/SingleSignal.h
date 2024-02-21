#pragma once

#include "prerequisites.h"

namespace waterside
{
	template<typename R, typename ...Args>
	class TSingleSignalSlot
	{
	public:
		TSingleSignalSlot() = default;

		virtual ~TSingleSignalSlot() = default;

		virtual R invoke(Args... args) = 0;
	};


	template<typename R, typename ...Args>
	class TStaticSingleSignalSlot : public TSingleSignalSlot<R, Args...>
	{
	public:
		typedef std::function<R(Args...)> CallbackType;

		TStaticSingleSignalSlot(const CallbackType& callback)
			: mCallback(callback)
		{
		}

		virtual R invoke(Args... args) override
		{
			return mCallback(std::forward<Args>(args)...);
		}
	protected:
		CallbackType mCallback;
	};

	template<typename ...Args>
	class TStaticSingleSignalSlot<void, Args...> : public TSingleSignalSlot<void, Args...>
	{
	public:
		typedef std::function<void(Args...)> CallbackType;

		TStaticSingleSignalSlot(const CallbackType& callback)
			: mCallback(callback)
		{
		}

		virtual void invoke(Args... args) override
		{
			mCallback(std::forward<Args>(args)...);
		}
	protected:
		CallbackType mCallback;
	};

	template<typename T, typename R, typename ...Args>
	class TSharedSingleSignalSlot : public TSingleSignalSlot<R, Args...>
	{
	public:
		typedef std::function<R(T*, Args...)> CallbackType;

		TSharedSingleSignalSlot(std::shared_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual R invoke(Args... args) override
		{
			if (mPtr)
			{
				return mCallback(mPtr.get(), std::forward<Args>(args)...);
			}
			return R{};
		}
	protected:
		std::shared_ptr<T> mPtr;
		CallbackType mCallback;
	};

	template<typename T, typename ...Args>
	class TSharedSingleSignalSlot<T, void, Args...> : public TSingleSignalSlot<void, Args...>
	{
	public:
		typedef std::function<void(T*, Args...)> CallbackType;

		TSharedSingleSignalSlot(std::shared_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual void invoke(Args... args) override
		{
			if (mPtr)
			{
				mCallback(mPtr.get(), std::forward<Args>(args)...);
			}
		}
	protected:
		std::shared_ptr<T> mPtr;
		CallbackType mCallback;
	};

	template<typename T, typename R, typename ...Args>
	class TWeakSingleSignalSlot : public TSingleSignalSlot<R, Args...>
	{
	public:
		typedef std::function<R(T*, Args...)> CallbackType;

		TWeakSingleSignalSlot(std::weak_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual R invoke(Args... args) override
		{
			if (auto p = mPtr.lock(); p)
			{
				return mCallback(p.get(), std::forward<Args>(args)...);
			}
			return R{};
		}
	protected:
		std::weak_ptr<T> mPtr;
		CallbackType mCallback;
	};

	template<typename T, typename ...Args>
	class TWeakSingleSignalSlot<T, void, Args...> : public TSingleSignalSlot<void, Args...>
	{
	public:
		typedef std::function<void(T*, Args...)> CallbackType;

		TWeakSingleSignalSlot(std::weak_ptr<T> ptr, const CallbackType& callback)
			: mPtr(ptr)
			, mCallback(callback)
		{
		}

		virtual void invoke(Args... args) override
		{
			if (auto p = mPtr.lock(); p)
			{
				mCallback(p.get(), std::forward<Args>(args)...);
			}
		}
	protected:
		std::weak_ptr<T> mPtr;
		CallbackType mCallback;
	};


	template<typename Signature> class TSingleSignal;

	template<typename R, typename ...Args>
	class TSingleSignal<R(Args...)>
	{
	public:
		void connect(const std::function<R(Args...)>& callback)
		{
			mSlot = std::make_shared<TStaticSingleSignalSlot<R, Args...>>(callback);
		}

		template<typename T>
		void connect(std::shared_ptr<T> ptr, const std::function<R(T*, Args...)>& callback)
		{
			mSlot = std::make_shared<TSharedSingleSignalSlot<T, R, Args...>>(ptr, callback);
		}

		template<typename T>
		void connect(std::weak_ptr<T> ptr, const std::function<R(T*, Args...)>& callback)
		{
			mSlot = std::make_shared<TWeakSingleSignalSlot<T, R, Args...>>(ptr, callback);
		}

		void disconnect()
		{
			mSlot.reset();
		}

		R emit(Args... args)
		{
			if (mSlot)
			{
				return mSlot->invoke(std::forward<Args>(args)...);
			}
			return R{};
		}
	private:
		std::shared_ptr<TSingleSignalSlot<R, Args...>> mSlot;
	};

	template<typename ...Args>
	class TSingleSignal<void(Args...)>
	{
	public:
		void connect(const std::function<void(Args...)>& callback)
		{
			mSlot = std::make_shared<TStaticSingleSignalSlot<void, Args...>>(callback);
		}

		template<typename T>
		void connect(std::shared_ptr<T> ptr, const std::function<void(T*, Args...)>& callback)
		{
			mSlot = std::make_shared<TSharedSingleSignalSlot<T, void, Args...>>(ptr, callback);
		}

		template<typename T>
		void connect(std::weak_ptr<T> ptr, const std::function<void(T*, Args...)>& callback)
		{
			mSlot = std::make_shared<TWeakSingleSignalSlot<T, void, Args...>>(ptr, callback);
		}

		void disconnect()
		{
			mSlot.reset();
		}

		bool isValid() const
		{
			return !!mSlot;
		}
		operator bool() const
		{
			return isValid();
		}

		void emit(Args... args)
		{
			if (mSlot)
			{
				mSlot->invoke(std::forward<Args>(args)...);
			}
		}
	private:
		std::shared_ptr<TSingleSignalSlot<void, Args...>> mSlot;
	};
}
