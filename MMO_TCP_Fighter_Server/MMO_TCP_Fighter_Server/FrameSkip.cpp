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
	mLoopCounter = 0;
	mMaxFramePeriod = 0;
	mMinFramePeriod = -1;
	mOldFrameTime = 0;
}

void FrameSkip::UpdateRemain()
{
	ULONGLONG framePeriod = mTotalTick - mOldFrameTime;
	mOldFrameTime = mTotalTick;
	if (framePeriod > mMaxFramePeriod)
	{
		mMaxFramePeriod = framePeriod;
	}

	if (framePeriod < mMinFramePeriod)
	{
		mMinFramePeriod = framePeriod;
	}
	mTimeRemain -= 20;
	mFrameCounter++;
}

void FrameSkip::PrintStatus()
{
	wprintf_s(L"-----------------------------------\n");
	wprintf_s(L"Frame Count: %d\n", mFrameCounter);
	wprintf_s(L"Loop Count: %d\n", mLoopCounter);
	wprintf_s(L"Max Frame Period: %u\n", (unsigned int)mMaxFramePeriod);
	wprintf_s(L"Min Frame Period: %u\n", (unsigned int)mMinFramePeriod);
	wprintf_s(L"-----------------------------------\n");
}
