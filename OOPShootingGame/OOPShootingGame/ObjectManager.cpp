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

void ObjectManager::Destroy()
{
	if (mManager != nullptr)
		delete mManager;
}

ObjectManager::~ObjectManager()
{
	while (mObjectList.empty() == false)
	{
		ObjectBase* cur = mObjectList.pop_front();
		delete cur;
	}

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

	for (int i = 0; i < mObjectSize; ++i)
	{
		char buffer[64] = { 0, };

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			memcpy(mObjectStats[i].name, buffer, sizeof(buffer));
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].hp = atoi(buffer);
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].damage = atoi(buffer);
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].image = *buffer;
		}
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
