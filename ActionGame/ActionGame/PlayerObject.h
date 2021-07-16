#pragma once

#include "BaseObject.h"

class PlayerObject : public BaseObject
{
public:
	PlayerObject();
	virtual ~PlayerObject();
	virtual void Render(BYTE* pDest, int destWidth, int destHeight, int destPitch, COORD* camera);
	virtual void Run();
	void Action();
	void ActionProc();
	void ActionPlayerProc();
	void ActionEnemyProc();
	void SetDirection(DWORD dir);
	void SetActionAttack1(bool sendMsg = true);
	void SetActionAttack2(bool sendMsg = true);
	void SetActionAttack3(bool sendMsg = true);
	void CreateEffect();
	void CreateEffectMySelf();
	void SetActionMove(bool sendMsg = true);
	void SetActionStand(bool sendMsg = true);
	void SetHP(char hp) { mHP = hp; }
	char GetHP() { return mHP; }
	bool IsLeft() { return mbIsLeft; }
	void Move();
	void SetID(int id) { mObjectID = id; }
	bool GetOldAction() const { return mActionOld; }

protected:
	bool IsAttackAction(DWORD action) const;

private:
	bool mbIsLeft;
	char mHP;
	int mDirCur;
	int mDirOld;
	DWORD mActionCur;
	DWORD mActionOld;
};