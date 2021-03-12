#pragma once
#include "ObjectManager.h"
#include "ObjectBase.h"
#include "operatorNewOverload.h"
#include "CSVReader.h"
#include <memory.h>
#include <stdlib.h>
#include "ObjectType.h"
#include "SceneManager.h"

ObjectManager* ObjectManager::mManager = nullptr;
ObjectStat* ObjectManager::mObjectStats = nullptr;

ObjectManager* ObjectManager::GetInstance()
{
	if (mManager == nullptr)
	{
		mManager = new ObjectManager;
	}

	return mManager;
}

void ObjectManager::AddObject(ObjectBase* object)
{
	mObjectList.push_back(object);
}

ObjectBase* ObjectManager::GetUnitAt(int x, int y, ObjectType type)
{
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		if ((*iter)->mX == x && (*iter)->mY == y && (*iter)->mType == type)
		{
			return *iter;
		}
	}

	return nullptr;
}

void ObjectManager::ClearObjects()
{
	while (mObjectList.empty() == false)
	{
		ObjectBase* current = mObjectList.pop_front();

		delete current;
	}
}

void ObjectManager::Update()
{
	// Update
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		(*iter)->Update();
	}

	// Delete
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end();)
	{
		if ((*iter)->GetIsLive() == false)
		{
			ObjectBase* pObj = *iter;
			iter = mObjectList.erase(iter);

			if (pObj->mType == ObjectType::PLAYER)
			{
				SceneManager::mbChangeScene = true;
				SceneManager::mNextSceneType = SceneType::SCENE_OVER;
			}

			delete pObj;
		}
		else
		{
			++iter;
		}
	}

	// Count Enemy
	int count = 0;

	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		if ((*iter)->mType == ObjectType::ENEMY)
		{
			++count;
		}
	}

	if (count == 0)
	{
		SceneManager::mbNextStage = true;
	}
}

void ObjectManager::Render()
{
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		(*iter)->Render();
	}
}

ObjectManager::~ObjectManager()
{
	while (mObjectList.empty() == false)
	{
		ObjectBase* cur = mObjectList.pop_front();
		delete cur;
	}

	if (mManager != nullptr)
		delete mManager;

	if (mObjectStats != nullptr)
		delete[] mObjectStats;
}

ObjectManager::ObjectManager(const char* fileName)
{
	LoadCSVFile(fileName);
}

void ObjectManager::LoadCSVFile(const char* fileName)
{
	CSVFile csvFile(fileName);

	int mObjectSize = csvFile.GetRow();
	int mObjectColSize = csvFile.GetCol();

	mObjectStats = new ObjectStat[mObjectSize];

	const char* pBegin = csvFile.GetRowAddress(1);
	const char* pEnd = pBegin;

	for (int i = 0; i < mObjectSize; ++i)
	{
		csvFile.GetNextComma(&pEnd);
		int size = pEnd - pBegin;
		char buffer[64];

		memcpy(mObjectStats[i].name, pBegin, size);
		mObjectStats[i].name[size] = '\0';

		++pEnd;
		pBegin = pEnd;

		csvFile.GetNextComma(&pEnd);

		size = pEnd - pBegin;
		memcpy(buffer, pBegin, size);
		buffer[size] = '\0';

		mObjectStats[i].hp = atoi(buffer);

		++pEnd;
		pBegin = pEnd;

		csvFile.GetNextComma(&pEnd);

		size = pEnd - pBegin;
		memcpy(buffer, pBegin, size);
		buffer[size] = '\0';

		mObjectStats[i].damage = atoi(buffer);

		++pEnd;

		mObjectStats[i].image = *pEnd;

		pEnd += 2;
		pBegin = pEnd;
	}
}

void ObjectManager::ClearNonePlayerObjects()
{
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		if ((*iter)->mType != ObjectType::PLAYER)
		{
			ObjectBase* pObj = *iter;
			mObjectList.erase(iter);

			delete pObj;
		}
	}
}
