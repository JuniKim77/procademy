#pragma once
#include "EObjectType.h"
#include "myList.h"
#include <Windows.h>

class ScreenDib;
class SpriteDib;

extern ScreenDib gScreenDib;
extern SpriteDib gSpriteDib;

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
	virtual void Render(BYTE* pDest, int destWidth, int destHeight, int destPitch) = 0;
	virtual void Run() = 0;
	void SetPosition(int x, int y);
	void SetSprite(int spriteBegin, int spriteEnd, int frameDelay);
	bool IsPlayer() { return mbPlayerCharacter; }
	void SetEnemy() { mbPlayerCharacter = false; }
	bool GetDestroy() { return mbDestroy; }
	void SetDestroy() { mbDestroy = true; }

protected:
	int mCurX = 0;
	int mCurY = 0;
	int mDelayCount = 0;
	int mFrameDelay = 0;
	int mObjectID;
	EObjectType mObjectType;
	int mSpriteEnd;
	int mSpriteNow;
	int mSpriteBegin;
	bool mbEndFrame;
	bool mbPlayerCharacter;
	DWORD mActionInput;
	bool mbDestroy = false;
};

extern BaseObject* gPlayerObject;
extern myList<BaseObject*> gObjectList;