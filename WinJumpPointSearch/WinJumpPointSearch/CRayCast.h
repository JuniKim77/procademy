#pragma once
#include "JumpPoint.h"

class CRayCast
{
public:
	CRayCast() {}
	void SetBegin(Coordi begin);
	void SetEnd(Coordi end);
	bool GetNext(Coordi& next);
	void Reset();

private:
	Coordi mBegin;
	Coordi mEnd;
	Coordi mCur;
	int mCount;
	int mdx;
	int mdy;
	int mNum;
	bool mXAxis;
	char mXMove;
	char mYMove;
};