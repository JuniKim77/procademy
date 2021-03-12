#pragma once
#include "ObjectBase.h"
#include "ObjectEnemy.h"
#include "ObjectManager.h"
#include "ObjectBullet.h"
#include "operatorNewOverload.h"
#include "ObjectType.h"

ObjectEnemy::ObjectEnemy(int x, int y, int type)
	: ObjectBase(x, y, 
		ObjectManager::mObjectStats[type].hp,
		ObjectManager::mObjectStats[type].damage,
		ObjectManager::mObjectStats[type].image, ObjectType::ENEMY)
	, mMoveTimer(20)
	, mAttackTimer(50)
{
	
}

bool ObjectEnemy::Update()
{
	++mAttackCount;
	++mMoveCounter;

	Move();

	Attack();

	return false;
}

void ObjectEnemy::Render()
{
	szScreenBuffer[mY][mX] = mImage;
}

void ObjectEnemy::Move()
{
	if (mMoveCounter % mMoveTimer == 0)
	{
		mX += mMoveDir;
	}

	if (mMoveCounter / mMoveTimer == 3)
	{
		mMoveDir = -mMoveDir;
		mMoveCounter = 0;
	}
}

void ObjectEnemy::Attack()
{
	if (mAttackCount == mAttackTimer)
	{
		mAttackCount = 0;
		ObjectManager::GetInstance()->AddObject(new ObjectBullet(mX, mY + 1, mAttack, true));
	}
}
