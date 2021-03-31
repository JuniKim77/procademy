#include "SpriteDib.h"
#include <stdio.h>

SpriteDib::SpriteDib(int maxSprite, DWORD colorKey)
	: mMaxSprite(maxSprite)
	, mColorKey(colorKey)
	, mpSprite(nullptr)
{
	mpSprite = new Sprite[mMaxSprite];
	memset(mpSprite, 0, sizeof(Sprite) * mMaxSprite);
}

SpriteDib::~SpriteDib()
{
	for (int i = 0; i < mMaxSprite; ++i)
	{
		ReleaseSprite(i);
	}
}

bool SpriteDib::LoadDibSprite(int spriteIndex, const char* fileName, int centerPointX, int centerPointY)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	FILE* fin;

	fopen_s(&fin, fileName, "rb");

	ReleaseSprite(spriteIndex);

	fread_s(&fileHeader, sizeof(BITMAPFILEHEADER), sizeof(BITMAPFILEHEADER), 1, fin);

	if (fileHeader.bfType == 0x4d42)
	{
		fread_s(&infoHeader, sizeof(BITMAPINFOHEADER), sizeof(BITMAPINFOHEADER), 1, fin);
		if (infoHeader.biBitCount == 32)
		{
			int pitch = infoHeader.biWidth * 4;

			mpSprite[spriteIndex].width = infoHeader.biWidth;
			mpSprite[spriteIndex].height = infoHeader.biHeight;
			mpSprite[spriteIndex].pitch = pitch;

			int imageSize = pitch * infoHeader.biHeight;
			mpSprite[spriteIndex].pImage = new BYTE[imageSize];

			BYTE* buffer = new BYTE[imageSize];
			BYTE* pSprite = mpSprite[spriteIndex].pImage;
			BYTE* pBuf = buffer + pitch * (infoHeader.biHeight - 1);

			fread_s(buffer, imageSize, imageSize, 1, fin);

			for (int i = 0; i < infoHeader.biHeight; ++i)
			{
				memcpy(pSprite, pBuf, pitch);
				pBuf -= pitch;
				pSprite += pitch;
			}

			mpSprite[spriteIndex].centerPointX = centerPointX;
			mpSprite[spriteIndex].centerPointY = centerPointY;
			delete[] buffer;

			fclose(fin);
			return true;
		}
	}
	fclose(fin);

	return false;
}

void SpriteDib::ReleaseSprite(int spriteIndex)
{
	if (spriteIndex < 0 || spriteIndex >= mMaxSprite)
		return;

	if (mpSprite[spriteIndex].pImage != nullptr)
	{
		delete[] mpSprite[spriteIndex].pImage;
		memset(&mpSprite[spriteIndex], 0, sizeof(Sprite));
	}
}

void SpriteDib::DrawSprite(int spriteIndex, int drawX, int drawY, BYTE* dest, int destWidth, int destHeight, int destPitch, int drawLen)
{
}

void SpriteDib::DrawImage(int spriteIndex, int drawX, int drawY, BYTE* dest, int destWidth, int destHeight, int destPitch, int drawLen)
{
}

void SpriteDib::Draw(int spriteIndex, int drawX, int drawY, BYTE* dest, int destWidth, int destHeight, int destPitch, int drawLen, bool colorKey)
{
	if (spriteIndex < 0 || spriteIndex >= mMaxSprite)
		return;

	if (mpSprite[spriteIndex].pImage == nullptr)
		return;

	Sprite* pSprite = &mpSprite[spriteIndex];

	int SpriteWidth = pSprite->width;
	int SpriteHeight = pSprite->height;

	SpriteWidth = SpriteWidth * drawLen / 100;

	DWORD* pDest = (DWORD*)dest;
	DWORD* pSpriteImage = (DWORD*)pSprite->pImage;

	drawX -= pSprite->centerPointX;
	drawY -= pSprite->centerPointY;

	// ��� Ŭ���� ó��
	if (drawY < 0)
	{
		SpriteHeight += drawY;
		pSpriteImage = (DWORD*)(pSprite->pImage + pSprite->pitch * (-drawY));

		drawY = 0;
	}

	// �ϴ� Ŭ����
	if ((drawY + pSprite->height) >= destHeight)
	{
		SpriteHeight -= (drawY + pSprite->height) - destHeight;
	}

	// ���� Ŭ����
	if (drawX < 0)
	{
		SpriteWidth += drawX;
		pSpriteImage += (-drawX);
		drawX = 0;
	}

	// ������ Ŭ����
	if ((drawX + pSprite->width) >= destWidth)
	{
		SpriteWidth -= ((drawX + pSprite->width) - destWidth);
	}

	if (SpriteHeight <= 0 || SpriteWidth <= 0)
		return;

	pDest = (DWORD*)((BYTE*)(pDest + drawX) + drawY * destPitch);

	if (colorKey)
	{
		for (int i = 0; i < SpriteHeight; ++i)
		{
			for (int j = 0; j < SpriteWidth; ++j)
			{
				if ((*pSpriteImage & 0x00ffffff) != mColorKey)
				{
					*pDest = *pSpriteImage;
				}

				pDest++;
				pSpriteImage++;
			}

			pDest = (DWORD*)((BYTE*)pDest + destPitch) - SpriteWidth;
			pSpriteImage = (DWORD*)((BYTE*)pSpriteImage + pSprite->pitch) - SpriteWidth;
		}
	}
	else
	{
		for (int i = 0; i < SpriteHeight; ++i)
		{
			memcpy(pDest, pSpriteImage, SpriteWidth * 4);

			pDest = (DWORD*)((BYTE*)pDest + destPitch);
			pSpriteImage = (DWORD*)((BYTE*)pSpriteImage + pSprite->pitch);
		}
	}
}
