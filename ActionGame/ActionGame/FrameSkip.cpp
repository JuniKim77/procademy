#include "FrameSkip.h"

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
	int curTick = timeGetTime();
	
	int timePeriod = curTick - mOldTick;

	mTotalTick += timePeriod;
	mFrameCounter++;

	mTimeRemain += (timePeriod - 20);
}

void FrameSkip::RunSleep()
{
	if (mTimeRemain < 0)
	{
		Sleep(20 + mTimeRemain);
	}
}

void FrameSkip::Reset()
{
	mTotalTick = 0;
	mFrameCounter = 0;
	mTimeRemain = 0;
	mOldTick = 0;
}
