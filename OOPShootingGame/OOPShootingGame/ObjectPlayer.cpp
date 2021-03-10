#pragma once
#include <Windows.h>
#include "ObjectBase.h"
#include "ObjectPlayer.h"
#include "ObjectManager.h"
#include "ObjectBullet.h"
#include "operatorNewOverload.h"

ObjectPlayer::ObjectPlayer(int x, int y)
	: ObjectBase(x, y, '^', ObjectType::PLAYER)
	, mHp(5)
	, mAttack(1)
{
}

bool ObjectPlayer::Update()
{
	Move();

	if (GetAsyncKeyState(VK_SPACE) & 0x8001)
	{
		// �Ѿ� �߻�
		ObjectManager::GetInstance()->AddObject(new ObjectBullet(mX, mY - -1, mAttack, false));
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
