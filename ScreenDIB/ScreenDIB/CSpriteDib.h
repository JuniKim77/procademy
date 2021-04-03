#pragma once
#include <Windows.h>

class CSpriteDib
{
public:
	struct Sprite
	{
		BYTE* bypImage;
		int width;
		int height;
		int pitch;

		int centerPointX;
		int centerPointY;
	};

	CSpriteDib(int maxSprite, DWORD colorKey);
	virtual ~CSpriteDib();

private:
		
};