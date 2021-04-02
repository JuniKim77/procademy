#pragma once
#include "ObjectManager.h"
#include "ObjectBase.h"
#include "operatorNewOverload.h"
#include "CSVReader.h"
#include <memory.h>
#include <stdlib.h>
#include "ObjectType.h"
#include "SceneManager.h"
#include "GameGlobalData.h"

ObjectManager* ObjectManager::mManager = nullptr;

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
				GameGlobalData::GetInstance()->mbChangeScene = true;
				GameGlobalData::GetInstance()->mNextSceneType = SceneType::SCENE_OVER;
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
		GameGlobalData::GetInstance()->mbNextStage = true;
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
}

ObjectManager::ObjectManager()
{
}

void ObjectManager::ClearNonePlayerObjects()
{
	for (myList<ObjectBase*>::iterator iter = mObjectList.begin(); iter != mObjectList.end();)
	{
		if ((*iter)->mType != ObjectType::PLAYER)
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
}
