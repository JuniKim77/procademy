#pragma once
#include "EObjectType.h"
#include "myList.h"
#include <Windows.h>

class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();
	void ActionInput(DWORD action);
	int GetCurX() { return mCurX; }
	int GetCurY() { return mCurY; }
	int GetObectID() { return mObjectID; }
	int GetSprite() { return mSpriteNow; }
	bool IsEndFrame() { return mbEndFrame; }
	void NextFrame();
	virtual void Render();
	virtual void Run();
	void SetPosition(int x, int y);
	void SetSprite(int sprite);

private:
	int mCurX = 0;
	int mCurY = 0;
	int mDelayCount = 0;
	int mFrameDelay = 0;
	int mObjectID = 0;
	EObjectType mObjectType;
	int mSpriteEnd;
	int mSpriteNow;
	int mSpriteBegin;
	bool mbEndFrame;
	bool mActionInput;
};

extern BaseObject* gPlayerObject;
extern myList<BaseObject*> gObjectList;