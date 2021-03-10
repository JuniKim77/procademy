#ifndef OBJECTMANAGER
#define OBJECTMANAGER
#include "myList.h"

class ObjectBase;

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