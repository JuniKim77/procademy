#include "CDib.h"

CDib::CDib(int width, int height, int colorBit, bool inverse)
	: mWidth(width)
	, mHeight(height)
	, mColorBit(colorBit)
	, mBuffer(nullptr)
{
	CreateDibBuffer(width, height, colorBit, inverse);
}

CDib::~CDib()
{
	ReleaseDibBuffer();
}

void CDib::CreateDibBuffer(int width, int height, int colorBit, bool inverse)
{
	mDibInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	mDibInfo.bmiHeader.biWidth = width;
	if (inverse)
	{
		mDibInfo.bmiHeader.biHeight = -height;
	}
	else
	{
		mDibInfo.bmiHeader.biHeight = height;
	}
	mDibInfo.bmiHeader.biBitCount = colorBit;
	mDibInfo.bmiHeader.biPlanes = 1;
	mDibInfo.bmiHeader.biCompression = 0;
	mDibInfo.bmiHeader.biClrUsed = 0;
	mDibInfo.bmiHeader.biClrImportant = 0;

	mPitch = (width * (colorBit / 8) + 3) & ~3;
	mDibInfo.bmiHeader.biSizeImage = mPitch * height;

	mBuffer = new BYTE[mPitch * height];
}

void CDib::ReleaseDibBuffer()
{
	if (mBuffer != nullptr)
		delete[] mBuffer;
}

void CDib::Filp(HWND hWnd, int x, int y)
{
	HDC hdc = GetDC(hWnd);
	RECT rect;
	
	GetClientRect(hWnd, &rect);
	StretchDIBits(hdc, x, y, mWidth, mHeight,
		0, 0, mWidth, mHeight, mBuffer, &mDibInfo, DIB_RGB_COLORS, SRCCOPY);
}

BYTE* CDib::GetDibBuffer()
{
	return mBuffer;
}

int CDib::GetWidth()
{
	return mWidth;
}

int CDib::GetHeight()
{
	return mHeight;
}

int CDib::GetPitch()
{
	return mPitch;
}
