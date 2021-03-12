#pragma once
#include "ObjectBase.h"
#include "ObjectBullet.h"
#include "ObjectManager.h"
#include "ObjectType.h"

ObjectBullet::ObjectBullet(int x, int y, int damage, bool isEnemy)
	: ObjectBase(x, y, 0, damage, isEnemy ? '+' : '*', ObjectType::BULLET)
	, mbEnemy(isEnemy)
{
}

bool ObjectBullet::Update()
{
	Move();

	Collision();

	return false;
}

void ObjectBullet::Render()
{
	szScreenBuffer[mY][mX] = mImage;
}

void ObjectBullet::Move()
{
	if (mbEnemy == true)
	{
		mY = min(dfSCREEN_HEIGHT - 1, mY + 1);
	}
	else
	{
		mY = max(0, mY - 1);
	}
}

void ObjectBullet::Collision()
{
	// To Wall
	if (mY == 0 || mY == dfSCREEN_HEIGHT - 1)
	{
		mbLive = false;
	}

	// To other Object
	ObjectBase* target = ObjectManager::GetInstance()->GetUnitAt(mX, mY, mbEnemy ? ObjectType::PLAYER : ObjectType::ENEMY);

	if (target != nullptr)
	{
		target->TakeDamage(mAttack);
		mbLive = false;
	}
}
