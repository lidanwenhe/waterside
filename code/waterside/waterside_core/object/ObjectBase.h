#pragma once

#include "prerequisites.h"

namespace waterside
{
	struct ObjectClass
	{
		const char* lpszClassName;
		ObjectClass* pBaseClass;
	};

	class ObjectBase
	{
		ObjectBase(const ObjectBase&) = delete;
		const ObjectBase& operator =(const ObjectBase&) = delete;
	public:
		ObjectBase();

		virtual ~ObjectBase();

		unique_id getId() const
		{
			return mId;
		}
		void setId(unique_id id)
		{
			mId = id;
		}

		virtual ObjectClass* getRuntimeClass() const
		{
			return &_clazz;
		}

		static ObjectClass _clazz;
		template<typename T>
		T* cast()
		{
			return isKindOf(&T::_clazz) ? static_cast<T*>(this) : nullptr;
		}

		virtual void initialize() {}
		virtual void release();

	protected:
		bool isKindOf(const ObjectClass* pClass) const;

	private:
		unique_id mId = 0; // 对象唯一ID
	};
}

#define A_OBJECT_DECLARE(className) public: \
	static waterside::ObjectClass _clazz; \
	virtual waterside::ObjectClass* getRuntimeClass() const override;

#define A_OBJECT_IMPLEMENT(className, baseClassName) \
	waterside::ObjectClass className::_clazz = { #className, &baseClassName::_clazz }; \
	waterside::ObjectClass* className::getRuntimeClass() const { return &className::_clazz; }