#include "ObjectManager.h"

namespace waterside
{
	void ObjectManager::release()
	{
		mObjects.clear();
	}

	bool ObjectManager::onCreateObject(std::shared_ptr<ObjectBase> pObject)
	{
		return mObjects.emplace(pObject->getId(), pObject).second;
	}

	void ObjectManager::onDestroyObject(unique_id id)
	{
		mObjects.erase(id);
	}

	bool ObjectManager::containsObjectBase(unique_id id)
	{
		return mObjects.contains(id);
	}

	std::shared_ptr<ObjectBase> ObjectManager::findObjectBase(unique_id id)
	{
		if (auto it = mObjects.find(id); it != mObjects.end())
		{
			return it->second;
		}
		return nullptr;
	}
}
