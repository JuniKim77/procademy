#ifndef OBJECTMANAGER
#define OBJECTMANAGER
#include "myList.h"

class ObjectBase;
class CSVFile;
enum class ObjectType;

#define NAME_SIZE (64)

struct ObjectStat
{
	char name[NAME_SIZE];
	int hp;
	int damage;
	char image;
};

class ObjectManager
{
public:
	static ObjectManager* GetInstance();
	static ObjectStat* mObjectStats;
	void AddObject(ObjectBase* object);
	ObjectBase* GetUnitAt(int x, int y, ObjectType type);
	void ClearObjects();
	void ClearNonePlayerObjects();
	void Update();
	void Render();
	void Destroy();

private:
	~ObjectManager();
	ObjectManager(const char* fileName = "unit_stat.csv");
	void LoadCSVFile(const char* fileName);

private:
	static ObjectManager* mManager;
	myList<ObjectBase*> mObjectList;
};

#endif