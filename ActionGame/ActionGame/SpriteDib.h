#pragma once

#include <Windows.h>

class SpriteDib
{
public:
	enum DrawStyle
	{
		DRAW_STYLE_CLIPPING_ONLY,
		DRAW_STYLE_COLOR_KEY,
		DRAW_STYLE_REDDISH,
		DRAW_STYLE_TRANSLUCENT,
	};

	struct Sprite
	{
		BYTE* pImage;
		int height;
		int width;
		int pitch;

		int centerPointX;
		int centerPointY;
	};

	SpriteDib(int maxSprite, DWORD colorKey);
	virtual ~SpriteDib();
	bool LoadDibSprite(int spriteIndex, const char* fileName, int centerPointX, int centerPointY);
	void ReleaseSprite(int spriteIndex);
	// 努府俏 贸府, 漠扼虐 贸府
	void DrawSprite(int spriteIndex, int drawX, int drawY, BYTE* dest,
		int destWidth, int destHeight, int destPitch, int drawLen = 100);
	// 努府俏 贸府
	void DrawImage(int spriteIndex, int drawX, int drawY, BYTE* dest,
		int destWidth, int destHeight, int destPitch, int drawLen = 100);
	void DrawSpriteRed(int spriteIndex, int drawX, int drawY, BYTE* dest,
		int destWidth, int destHeight, int destPitch, int drawLen = 100);
	void DrawSprite50(int spriteIndex, int drawX, int drawY, BYTE* dest,
		int destWidth, int destHeight, int destPitch, int drawLen = 100);

protected:
	int mMaxSprite;
	Sprite* mpSprite;
	DWORD mColorKey;

protected:
	void Draw(int spriteIndex, int drawX, int drawY, BYTE* dest,
		int destWidth, int destHeight, int destPitch, DrawStyle style, int drawLen = 100);
};