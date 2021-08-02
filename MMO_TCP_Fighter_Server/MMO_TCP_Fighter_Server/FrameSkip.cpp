#include "FrameSkip.h"
#include <stdio.h>
#include <Windows.h>
#include "CLogger.h"

extern CLogger g_Logger;

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
	mPrevTime = 0;
	mFrameCounter = 0;
	mTimeRemain = 0;
	mMaxFrameTime = 0;
	mMinFrameTime = -1;
}

void FrameSkip::Refresh()
{
	mOldFrameCounter = mFrameCounter;
	mFrameCounter = 0;
	mTotalTick -= 1000;
	mPrevTime -= 1000;
	mLoopCounter = 0;
	mMaxFrameTime = 0;
	mMinFrameTime = LLONG_MAX;
}

void FrameSkip::UpdateRemain()
{
	mTimeRemain -= 20;
	mFrameCounter++;
	ULONGLONG framePeriod = mTotalTick - mPrevTime;
	mPrevTime = mTotalTick;

	if (framePeriod > mMaxFrameTime)
	{
		mMaxFrameTime = framePeriod;
	}
	if (framePeriod < mMinFrameTime)
	{
		mMinFrameTime = framePeriod;
	}
}

void FrameSkip::PrintStatus()
{
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[Frame Count: %d][Loop Count: %d]\n[Max Frame: %ums][Min Frame: %ums]\n",
		mFrameCounter, mLoopCounter, (unsigned int)mMaxFrameTime, (unsigned int)mMinFrameTime);
}


