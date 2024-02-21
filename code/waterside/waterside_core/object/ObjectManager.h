#pragma once

#include "ObjectBase.h"
#include "SnowflakeId.h"

namespace waterside
{
	class ObjectManager : public TLazySingleton<ObjectManager>
	{
	public:
		void release();

		template<typename T>
		std::shared_ptr<T> createObject(unique_id id)
		{
			auto p = std::make_shared<T>();
			p->setId(id);
			if (onCreateObject(p))
			{
				p->initialize();
				return p;
			}
			return nullptr;
		}
		template<typename T>
		std::shared_ptr<T> createObject()
		{
			auto p = std::make_shared<T>();
			p->setId(SnowflakeId::instance().nextId());
			if (onCreateObject(p))
			{
				p->initialize();
				return p;
			}
			return nullptr;
		}

		bool onCreateObject(std::shared_ptr<ObjectBase> pObject);
		void onDestroyObject(unique_id id);

		bool containsObjectBase(unique_id id);
		std::shared_ptr<ObjectBase> findObjectBase(unique_id id);
		template<typename T>
		std::shared_ptr<T> findObject(unique_id id)
		{
			auto p = findObjectBase(id);
			if (p->cast<T>())
				return std::static_pointer_cast<T>(p);
			return nullptr;
		}
	private:
		unordered_map<unique_id, std::shared_ptr<ObjectBase>> mObjects;
	};
}
