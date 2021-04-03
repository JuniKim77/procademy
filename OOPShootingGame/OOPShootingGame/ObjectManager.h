#ifndef OBJECTMANAGER
#define OBJECTMANAGER
#include "myList.h"

class ObjectBase;
class CSVFile;
enum class ObjectType;

class ObjectManager
{
public:
	static ObjectManager* GetInstance();
	void AddObject(ObjectBase* object);
	ObjectBase* GetUnitAt(int x, int y, ObjectType type);
	void ClearObjects();
	void ClearNonePlayerObjects();
	void Update();
	void Render();
	static void Destroy();

private:
	~ObjectManager();
	ObjectManager();

private:
	static ObjectManager* mManager;
	myList<ObjectBase*> mObjectList;
};

#endif