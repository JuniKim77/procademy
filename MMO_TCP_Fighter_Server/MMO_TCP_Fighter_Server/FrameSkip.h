#pragma once

#include <wtypes.h>

class FrameSkip
{
public:
	FrameSkip();
	~FrameSkip();
	bool IsSkip();
	ULONGLONG GetTotalTick() { return mTotalTick; }
	void CheckTime();
	int GetFrameCount() { return mFrameCounter; }
	void RunSleep();
	void Reset();
	void Refresh();
	void UpdateRemain();
	ULONGLONG GetOldFrameCount() { return mOldFrameCounter; }
	void AddLoopCounter() { mLoopCounter++; }
	int GetLoopCounter() { return mLoopCounter; }
	void PrintStatus();

private:
	ULONGLONG mTotalTick; // 경과 시간
	ULONGLONG mOldFrameCounter = 0;
	ULONGLONG mTimeRemain; // 남은 시간 누적
	ULONGLONG mOldTick; // 이전 루프 시간
	ULONGLONG mOldFrameTime = 0; // 이전 루프 시간
	ULONGLONG mMaxFramePeriod = 0;
	ULONGLONG mMinFramePeriod = -1;
	int mFrameCounter; // 프레임 카운터
	int mLoopCounter = 0; // 게임 루프 카운터
};