#include "CFrameSkipper.h"
#include <stdio.h>

procademy::CFrameSkipper::CFrameSkipper()
{
}

procademy::CFrameSkipper::~CFrameSkipper()
{
}

void procademy::CFrameSkipper::CheckTime()
{
	ULONGLONG curTick = GetTickCount64();
	// 루프 돈 총 시간
	ULONGLONG timePeriod = curTick - mOldTick;

	mOldTick = curTick;

	mTotalTick += timePeriod;

	mTimeRemain += timePeriod;

	mLoopCounter++;
}

void procademy::CFrameSkipper::RunSleep()
{
	if (mTimeRemain < 0 && mTimeRemain > -mStandardTime)
	{
		Sleep(mStandardTime + mTimeRemain);
	}
	else if (mTimeRemain <= -mStandardTime)
	{
		Sleep(mStandardTime);
	}
}

void procademy::CFrameSkipper::Reset()
{
	mTotalTick = 0;
	mPrevTime = 0;
	mFrameCounter = 0;
	mTimeRemain = 0;
	mMaxFrameTime = 0;
	mMinFrameTime = LLONG_MAX;
}

void procademy::CFrameSkipper::Refresh()
{
	mOldFrameCounter = mFrameCounter;
	mFrameCounter = 0;
	mTotalTick -= 1000;
	mPrevTime -= 1000;
	mLoopCounter = 0;
	mMaxFrameTime = 0;
	mMinFrameTime = LLONG_MAX;
}

void procademy::CFrameSkipper::UpdateRemain()
{
	// 기준 시간 감소
	mTimeRemain -= mStandardTime;
	mFrameCounter++;
	ULONGLONG framePeriod = mTotalTick - mPrevTime;
	mPrevTime = mTotalTick;

	// 최대 최소 갱신
	if (framePeriod > mMaxFrameTime)
	{
		mMaxFrameTime = framePeriod;
	}
	if (framePeriod < mMinFrameTime)
	{
		mMinFrameTime = framePeriod;
	}
}

void procademy::CFrameSkipper::PrintStatus()
{
	wprintf_s(L"[Frame Count: %d][Loop Count: %d]\n[Max Frame: %ums][Min Frame: %ums]\n",
		mFrameCounter, mLoopCounter, (unsigned int)mMaxFrameTime, (unsigned int)mMinFrameTime);
}

void procademy::CFrameSkipper::SetMaxFrame(int frame)
{
	mStandardTime = 1000 / frame;
}
