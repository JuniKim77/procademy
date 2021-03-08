#ifndef OBJECTMANAGER
#define OBJECTMANAGER
#include "myList.h"
#include "ObjectBase.h"

class ObjectManager
{
public:
	static ObjectManager* GetInstance();
	void AddObject(ObjectBase* object);
	void ClearObjects();
	void Update();
	void Render();

private:
	~ObjectManager();
	ObjectManager();

private:
	static ObjectManager* mManager;
	myList<ObjectBase*> mObjectList;
};

#endif