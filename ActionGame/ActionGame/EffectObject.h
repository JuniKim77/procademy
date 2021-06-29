#pragma once

#include "BaseObject.h"

class EffectObject : public BaseObject
{
public:
	EffectObject();
	virtual ~EffectObject();
	virtual void Render(BYTE* pDest, int destWidth, int destHeight, int destPitch, COORD* camera);
	virtual void Run();

private:
	void CheckDistance();
	
private:
	bool mbEffectBegin;
	int mAttackID;
};