#include "BaseObject.h"
#include "ActionDefine.h"
#include "ESprite.h"
#include <stdio.h>

myList<BaseObject*> gObjectList;
BaseObject* gPlayerObject;
extern int gIDCounter;

BaseObject::BaseObject()
	: mbEndFrame(true)
	, mActionInput(dfAction_STAND)
	, mInAction(dfAction_NONE)
	, mbPlayerCharacter(true)
	, mObjectID(gIDCounter++)
{
}

BaseObject::~BaseObject()
{
}

void BaseObject::ActionInput(DWORD action)
{
	mActionInput = action;
}

void BaseObject::NextFrame()
{
	if (mSpriteBegin < 0)
		return;

	mDelayCount++;

	if (mDelayCount >= mFrameDelay)
	{
		mDelayCount = 0;
		mSpriteNow++;

		if (mSpriteNow > mSpriteEnd)
		{
			if (mObjectType == EObjectType::OBJECT_TYPE_EFFECT)
			{
				mbDestroy = true;
			}
			else 
			{
				mInAction = dfAction_NONE;
			}

			mSpriteNow = mSpriteBegin;
			mbEndFrame = true;
		}
	}
}

void BaseObject::SetPosition(int x, int y)
{
	mCurX = x;
	mCurY = y;
}

void BaseObject::SetSprite(int spriteBegin, int spriteEnd, int frameDelay)
{
	mSpriteBegin = spriteBegin;
	mSpriteEnd = spriteEnd;
	mFrameDelay = frameDelay;

	mSpriteNow = spriteBegin;
	mDelayCount = 0;
	mbEndFrame = false;
}
