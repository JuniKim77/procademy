#pragma once

#include "BaseObject.h"

class PlayerObject : public BaseObject
{
public:
	PlayerObject();
	virtual ~PlayerObject();
	virtual void Render(BYTE* pDest, int destWidth, int destHeight, int destPitch);
	virtual void Run();
	void Action();
	void ActionInputProc();
	void ActionProc();
	void SetActionStand();
	void SetActionAttack1();
	void SetActionAttack2();
	void SetActionAttack3();
	void SetActionMove();
	void SetDirection(int dir) { mDir = dir; }
	void SetHP(char hp) { mHP = hp; }
	char GetHP() { return mHP; }
	int GetDirection() { return mDir; }
	bool IsLeft() { return mbIsLeft; }
	void Move();

private:
	bool mbIsLeft;
	char mHP;
	int mDir;
};