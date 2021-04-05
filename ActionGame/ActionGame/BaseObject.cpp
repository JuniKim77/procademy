#include "BaseObject.h"

myList<BaseObject*> gObjectList;
BaseObject* gPlayerObject;

BaseObject::BaseObject()
{
}

BaseObject::~BaseObject()
{
}

void BaseObject::ActionInput(DWORD action)
{
}

void BaseObject::NextFrame()
{
}

void BaseObject::Render()
{
}

void BaseObject::Run()
{
}

void BaseObject::SetPosition(int x, int y)
{
}

void BaseObject::SetSprite(int sprite)
{
}
