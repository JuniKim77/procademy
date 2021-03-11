#pragma once
#include "ObjectManager.h"
#include "ObjectBase.h"
#include "operatorNewOverload.h"
#include "CSVReader.h"
#include <memory.h>
#include <stdlib.h>

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
	// Delete
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end();)
	{
		if ((*iter)->GetIsLive() == false)
		{
			ObjectBase* pObj = *iter;
			iter = mObjectList.erase(iter);
			delete pObj;
		}
		else
		{
			++iter;
		}
	}

	// Update
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end(); ++iter)
	{
		(*iter)->Update();
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
