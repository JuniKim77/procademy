#pragma once
#include <Windows.h>

class ScreenDib 
{
public:
	ScreenDib(int width, int height, int colorBit);
	virtual ~ScreenDib();

protected:
	void CreateDibBuffer(int width, int height, int colorBit);
	void ReleaseDibBuffer();

public:
	void Filp(HWND hWnd, int x = 0, int y = 0);
	BYTE* GetDibBuffer();
	int GetWidth();
	int GetHeight();
	int GetPitch();

protected:
	BITMAPINFO mDibInfo;
	BYTE* mBuffer;
	int mWidth;
	int mHeight;
	int mPitch;
	int mColorBit;
	int mBufferSize;
};