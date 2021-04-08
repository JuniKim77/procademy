#pragma once
#include <Windows.h>

class FrameSkip
{
public:
	FrameSkip();
	~FrameSkip();
	bool IsSkip();
	DWORD GetTotalTick() { return mTotalTick; }
	void CheckTime();
	int GetFrameCount() { return mFrameCounter; }
	void RunSleep();
	void Reset();

private:
	DWORD mTotalTick; // 경과 시간
	int mFrameCounter; // 프레임 카운터
	int mTimeRemain; // 남은 시간 누적
	int mOldTick; // 이전 프레임 시간
};