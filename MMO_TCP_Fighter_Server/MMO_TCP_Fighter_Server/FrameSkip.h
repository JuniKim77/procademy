#pragma once

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
	void Refresh();
	int GetOldFrameCount() { return mOldFrameCounter; }

private:
	int mTotalTick; // ��� �ð�
	int mFrameCounter; // ������ ī����
	int mOldFrameCounter = 0;
	int mTimeRemain; // ���� �ð� ����
	int mOldTick; // ���� ������ �ð�
};