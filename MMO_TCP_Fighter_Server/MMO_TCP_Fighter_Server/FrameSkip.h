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

private:
	ULONGLONG mTotalTick; // ��� �ð�
	ULONGLONG mOldFrameCounter = 0;
	ULONGLONG mTimeRemain; // ���� �ð� ����
	ULONGLONG mOldTick; // ���� ������ �ð�
	int mFrameCounter; // ������ ī����
	int mLoopCounter = 0; // ���� ���� ī����
};