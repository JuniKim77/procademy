#pragma once
#include "ObjectBase.h"

ObjectBase::ObjectBase(int x, int y, int hp, int damage, char image, ObjectType type)
	: mX(x)
	, mY(y)
	, mHp(hp)
	, mAttack(damage)
	, mType(type)
	, mImage(image)
{
}

ObjectBase::~ObjectBase()
{

}

void ObjectBase::TakeDamage(int damage)
{
	mHp = max(0, mHp - damage);

	if (mHp == 0)
		mbLive = false;
}
