#pragma once
#include "ObjectBase.h"

ObjectBase::ObjectBase(int x, int y, char image, ObjectType type)
	: mX(x)
	, mY(y)
	, mType(type)
	, mImage(image)
{
}

ObjectBase::~ObjectBase()
{

}
