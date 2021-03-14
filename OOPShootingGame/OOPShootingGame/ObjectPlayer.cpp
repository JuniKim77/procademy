#pragma once
#include <Windows.h>
#include "ObjectBase.h"
#include "ObjectPlayer.h"
#include "ObjectManager.h"
#include "ObjectBullet.h"
#include "operatorNewOverload.h"
#include "ObjectType.h"
#include "GameGlobalData.h"

ObjectPlayer::ObjectPlayer(int x, int y)
	: ObjectBase(x, y, 
		GameGlobalData::GetInstance()->GetObjectStats()[(int)ObjectType::PLAYER].hp,
		GameGlobalData::GetInstance()->GetObjectStats()[(int)ObjectType::PLAYER].damage,
		GameGlobalData::GetInstance()->GetObjectStats()[(int)ObjectType::PLAYER].image,
		ObjectType::PLAYER)
{
}

bool ObjectPlayer::Update()
{
	Move();

	if (GetAsyncKeyState(VK_SPACE) & 0x8001)
	{
		// ÃÑ¾Ë ¹ß»ç
		ObjectManager::GetInstance()->AddObject(new ObjectBullet(mX, mY, mAttack, false));
	}

	return false;
}

void ObjectPlayer::Render()
{
	szScreenBuffer[mY][mX] = mImage;
}

void ObjectPlayer::Move()
{
	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		mY = max(0, mY - 1);
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		mY = min(dfSCREEN_HEIGHT - 1, mY + 1);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		mX = min(dfSCREEN_WIDTH - 2, mX + 1);
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		mX = max(0, mX - 1);
	}
}
