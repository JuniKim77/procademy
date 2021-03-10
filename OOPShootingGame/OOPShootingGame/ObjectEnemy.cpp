#pragma once
#include "ObjectBase.h"
#include "ObjectEnemy.h"
#include "ObjectManager.h"
#include "ObjectBullet.h"
#include "operatorNewOverload.h"

ObjectEnemy::ObjectEnemy(int x, int y)
	: ObjectBase(x, y, '#', ObjectType::ENEMY)
	, mHp(2)
	, mAttack(2)
	, mMoveTimer(10)
	, mAttackTimer(5)
{
}

bool ObjectEnemy::Update()
{
	//++mMoveCounter;
	++mAttackCount;

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
	
}

void ObjectEnemy::Attack()
{
	if (mAttackCount == mAttackTimer)
	{
		mAttackCount = 0;
		ObjectManager::GetInstance()->AddObject(new ObjectBullet(mX, mY + 1, mAttack, true));
	}
}
