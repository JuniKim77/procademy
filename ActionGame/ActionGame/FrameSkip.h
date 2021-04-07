#pragma once
#include <Windows.h>

class FrameSkip
{
public:
	FrameSkip();
	~FrameSkip();
	void Skip();

private:
	DWORD mSystemTick;
	int mMaxFPS;
	int mOneFrameTick;
	int mTick;
};