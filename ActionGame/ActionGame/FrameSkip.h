#pragma once
#include <Windows.h>

class FrameSkip
{
public:
	FrameSkip();
	~FrameSkip();
	bool IsSkip();
	int GetTotalTick() { return mTotalTick; }
	void CheckTime();
	int GetFrameCount() { return mFrameCounter; }
	void RunSleep();
	void Reset();

private:
	int mTotalTick; // ��� �ð�
	int mFrameCounter; // ������ ī����
	int mTimeRemain; // ���� �ð� ����
	int mOldTick; // ���� ������ �ð�
};