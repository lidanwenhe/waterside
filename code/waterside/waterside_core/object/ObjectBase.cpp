#include "ObjectBase.h"
#include "ObjectManager.h"

namespace waterside
{
	ObjectClass ObjectBase::_clazz = { "Object", nullptr };

	ObjectBase::ObjectBase()
	{
	}

	ObjectBase::~ObjectBase()
	{
	}

	void ObjectBase::release()
	{
		ObjectManager::instance().onDestroyObject(mId);
	}

	bool ObjectBase::isKindOf(const ObjectClass* pClass) const
	{
		ObjectClass* pClassThis = getRuntimeClass();
		while (pClassThis != nullptr)
		{
			if (pClassThis == pClass)
				return true;
			pClassThis = pClassThis->pBaseClass;
		}
		return false;
	}
}
