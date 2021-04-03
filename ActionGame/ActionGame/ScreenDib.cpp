#include "ScreenDib.h"

ScreenDib::ScreenDib(int width, int height, int colorBit)
	: mWidth(width)
	, mHeight(height)
	, mColorBit(colorBit)
	, mBuffer(nullptr)
{
	CreateDibBuffer(width, height, colorBit);
}

ScreenDib::~ScreenDib()
{
	ReleaseDibBuffer();
}

void ScreenDib::CreateDibBuffer(int width, int height, int colorBit)
{
	mPitch = (width * (colorBit / 8) + 3) & ~3;
	mBufferSize = mPitch * height;

	mDibInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mDibInfo.bmiHeader.biWidth = width;
	mDibInfo.bmiHeader.biHeight = -height;
	mDibInfo.bmiHeader.biBitCount = colorBit;
	mDibInfo.bmiHeader.biPlanes = 1;
	mDibInfo.bmiHeader.biCompression = 0;
	mDibInfo.bmiHeader.biSizeImage = mBufferSize;
	mDibInfo.bmiHeader.biXPelsPerMeter = 0;
	mDibInfo.bmiHeader.biYPelsPerMeter = 0;
	mDibInfo.bmiHeader.biClrUsed = 0;
	mDibInfo.bmiHeader.biClrImportant = 0;

	mBuffer = new BYTE[mBufferSize];
	memset(mBuffer, 0xff, mBufferSize);
}

void ScreenDib::ReleaseDibBuffer()
{
	if (mBuffer != nullptr)
		delete[] mBuffer;

	mBuffer = nullptr;
}

void ScreenDib::Filp(HWND hWnd, int x, int y)
{
	if (mBuffer == nullptr)
		return;

	HDC hdc = GetDC(hWnd);
	RECT rect;
	
	GetClientRect(hWnd, &rect);

	SetDIBitsToDevice(hdc, x, y, mWidth, mHeight,
		0, 0, 0, mHeight, mBuffer, &mDibInfo, DIB_RGB_COLORS);
}

BYTE* ScreenDib::GetDibBuffer()
{
	return mBuffer;
}

int ScreenDib::GetWidth()
{
	return mWidth;
}

int ScreenDib::GetHeight()
{
	return mHeight;
}

int ScreenDib::GetPitch()
{
	return mPitch;
}
