#pragma once
#include "ObjectManager.h"
#include "operatorNewOverload.h"

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
	if (mManager != nullptr)
		delete mManager;
}

ObjectManager::ObjectManager()
{
}
