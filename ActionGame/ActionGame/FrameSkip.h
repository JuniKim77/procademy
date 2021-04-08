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
	DWORD mTotalTick; // ��� �ð�
	int mFrameCounter; // ������ ī����
	int mTimeRemain; // ���� �ð� ����
	int mOldTick; // ���� ������ �ð�
};