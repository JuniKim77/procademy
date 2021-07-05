#include "FrameSkip.h"
#include <stdio.h>
#include <Windows.h>

FrameSkip::FrameSkip()
	: mTotalTick(0)
	, mFrameCounter(0)
	, mTimeRemain(0)
	, mOldTick(0)
{

}

FrameSkip::~FrameSkip()
{
}

bool FrameSkip::IsSkip()
{
	return mTimeRemain >= 20;
}

void FrameSkip::CheckTime()
{
	ULONGLONG curTick = GetTickCount64();
	
	ULONGLONG timePeriod = curTick - mOldTick;

	mOldTick = curTick;

	mTotalTick += timePeriod;

	mTimeRemain += timePeriod;
}

void FrameSkip::RunSleep()
{
	if (mTimeRemain < 0 && mTimeRemain > -20)
	{
		Sleep(20 + mTimeRemain);
	}
	else if (mTimeRemain <= -20)
	{
		Sleep(20);
	}
}

void FrameSkip::Reset()
{
	mTotalTick = 0;
	mFrameCounter = 0;
	mTimeRemain = 0;
}

void FrameSkip::Refresh()
{
	mOldFrameCounter = mFrameCounter;
	mFrameCounter = 0;
	mTotalTick -= 1000;
}

void FrameSkip::UpdateRemain()
{
	mTimeRemain -= 20;
}
