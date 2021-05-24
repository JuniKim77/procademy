#pragma once
#include "GameProcess.h"
#include "ScreenDib.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "SpriteDib.h"
#include "ESprite.h"
#include "ActionDefine.h"
#include <stdio.h>
#include "FrameSkip.h"

extern DWORD gOldTime;
Process gGameState = PROCESS_GAME;
extern FrameSkip gFrameSkipper;
extern HWND gMainWindow;
//int gIDCounter;

void InitializeGame()
{
	// 스프라이트 로드
	gSpriteDib.LoadDibSprite(eMAP, L"SpriteData\\_Map.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_L01, L"SpriteData\\Stand_L_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_L02, L"SpriteData\\Stand_L_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_L03, L"SpriteData\\Stand_L_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_R01, L"SpriteData\\Stand_R_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_R02, L"SpriteData\\Stand_R_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_STAND_R03, L"SpriteData\\Stand_R_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L01, L"SpriteData\\Move_L_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L02, L"SpriteData\\Move_L_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L03, L"SpriteData\\Move_L_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L04, L"SpriteData\\Move_L_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L05, L"SpriteData\\Move_L_05.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L06, L"SpriteData\\Move_L_06.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L07, L"SpriteData\\Move_L_07.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L08, L"SpriteData\\Move_L_08.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L09, L"SpriteData\\Move_L_09.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L10, L"SpriteData\\Move_L_10.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L11, L"SpriteData\\Move_L_11.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_L12, L"SpriteData\\Move_L_12.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R01, L"SpriteData\\Move_R_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R02, L"SpriteData\\Move_R_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R03, L"SpriteData\\Move_R_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R04, L"SpriteData\\Move_R_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R05, L"SpriteData\\Move_R_05.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R06, L"SpriteData\\Move_R_06.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R07, L"SpriteData\\Move_R_07.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R08, L"SpriteData\\Move_R_08.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R09, L"SpriteData\\Move_R_09.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R10, L"SpriteData\\Move_R_10.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R11, L"SpriteData\\Move_R_11.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_MOVE_R12, L"SpriteData\\Move_R_12.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_L01, L"SpriteData\\Attack1_L_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_L02, L"SpriteData\\Attack1_L_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_L03, L"SpriteData\\Attack1_L_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_L04, L"SpriteData\\Attack1_L_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_R01, L"SpriteData\\Attack1_R_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_R02, L"SpriteData\\Attack1_R_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_R03, L"SpriteData\\Attack1_R_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK1_R04, L"SpriteData\\Attack1_R_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_L01, L"SpriteData\\Attack2_L_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_L02, L"SpriteData\\Attack2_L_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_L03, L"SpriteData\\Attack2_L_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_L04, L"SpriteData\\Attack2_L_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_R01, L"SpriteData\\Attack2_R_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_R02, L"SpriteData\\Attack2_R_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_R03, L"SpriteData\\Attack2_R_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK2_R04, L"SpriteData\\Attack2_R_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L01, L"SpriteData\\Attack3_L_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L02, L"SpriteData\\Attack3_L_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L03, L"SpriteData\\Attack3_L_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L04, L"SpriteData\\Attack3_L_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L05, L"SpriteData\\Attack3_L_05.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_L06, L"SpriteData\\Attack3_L_06.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R01, L"SpriteData\\Attack3_R_01.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R02, L"SpriteData\\Attack3_R_02.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R03, L"SpriteData\\Attack3_R_03.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R04, L"SpriteData\\Attack3_R_04.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R05, L"SpriteData\\Attack3_R_05.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(ePLAYER_ATTACK3_R06, L"SpriteData\\Attack3_R_06.bmp", 71, 90);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_01, L"SpriteData\\xSpark_1.bmp", 70, 70);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_02, L"SpriteData\\xSpark_2.bmp", 70, 70);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_03, L"SpriteData\\xSpark_3.bmp", 70, 70);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_04, L"SpriteData\\xSpark_4.bmp", 70, 70);
	gSpriteDib.LoadDibSprite(eGUAGE_HP, L"SpriteData\\HPGuage.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eSHADOW, L"SpriteData\\Shadow.bmp", 32, 4);

	//// 플레이어 생성
	//gPlayerObject = new PlayerObject;
	//gPlayerObject->SetPosition(320, 240);
	//gObjectList.push_back(gPlayerObject);

	//// 테스트용 객체들
	//BaseObject* pObject = new PlayerObject;
	//pObject->SetPosition(100, 100);
	//pObject->SetEnemy();
	//gObjectList.push_back(pObject);

	//pObject = new PlayerObject;
	//pObject->SetPosition(125, 125);
	//pObject->SetEnemy();
	//gObjectList.push_back(pObject);

	//pObject = new PlayerObject;
	//pObject->SetPosition(150, 150);
	//pObject->SetEnemy();
	//gObjectList.push_back(pObject);
}

void ContentLoad()
{
}

void RunGame()
{
	switch (gGameState)
	{
	case PROCESS_TITLE:
		break;
	case PROCESS_GAME:
		UpdateGame();
		break;
	default:
		break;
	}
}

void UpdateGame()
{
	if (gbActiveApp)
	{
		KeyProcess();
	}
	Update(); // 객체 run, y축 좌표 기준 정렬, 

	gFrameSkipper.CheckTime();

	if (!gFrameSkipper.IsSkip())
	{
		Render(); // 백버퍼에 출력
	}

	gScreenDib.Filp(gMainWindow); // 윈도에 출력

	if (gFrameSkipper.GetTotalTick() >= 1000)
	{
		gFrameSkipper.Refresh();
		WCHAR msg[32] = { 0, };
		swprintf_s(msg, L"LogicFrame:%d, ID: %d", gFrameSkipper.GetOldFrameCount(), gPlayerObject->GetObectID());
		SetWindowText(gMainWindow, msg);
	}

	// 순수 Sleep 시간 체크
	gFrameSkipper.RunSleep();
}

void KeyProcess()
{
	if (gPlayerObject == nullptr)
	{
		return;
	}

	DWORD action = dfAction_STAND;

	if (GetAsyncKeyState(VK_RIGHT) & 0x8001 && GetAsyncKeyState(VK_UP) & 0x8001)
	{
		action = dfACTION_MOVE_RU;
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8001 && GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		action = dfACTION_MOVE_RD;
	}
	else if (GetAsyncKeyState(VK_LEFT) & 0x8001 && GetAsyncKeyState(VK_UP) & 0x8001)
	{
		action = dfACTION_MOVE_LU;
	}
	else if (GetAsyncKeyState(VK_LEFT) & 0x8001 && GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		action = dfACTION_MOVE_LD;
	}
	else if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		action = dfACTION_MOVE_RR;
	}
	else if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		action = dfACTION_MOVE_LL;
	}
	else if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		action = dfACTION_MOVE_UU;
	}
	else if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		action = dfACTION_MOVE_DD;
	}
	
	if (GetAsyncKeyState(0x5A) & 0x8001) // Z key
	{
		action = dfACTION_ATTACK1;
	}
	else if (GetAsyncKeyState(0x58) & 0x8001) // X key
	{
		action = dfACTION_ATTACK2;
	}
	else if (GetAsyncKeyState(0x43) & 0x8001) // C key
	{
		action = dfACTION_ATTACK3;
	}

	gPlayerObject->ActionInput(action);
}

void Update()
{
	for (auto iter = gObjectList.begin(); iter != gObjectList.end();)
	{
		if ((*iter)->GetDestroy())
		{
			BaseObject* temp = *iter;

			iter = gObjectList.erase(iter);			
			delete temp;
		}
		else
		{
			(*iter)->Run();
			++iter;
		}
	}

	SortYaxis();
}

void Render()
{
	gSpriteDib.DrawImage(eMAP, 0, 0, gScreenDib.GetDibBuffer(), gScreenDib.GetWidth(), gScreenDib.GetHeight(),
		gScreenDib.GetPitch());

	for (auto iter = gObjectList.begin(); iter != gObjectList.end(); iter++)
	{
		(*iter)->Render(gScreenDib.GetDibBuffer(), gScreenDib.GetWidth(), gScreenDib.GetHeight(),
			gScreenDib.GetPitch());
	}
}

void SortYaxis()
{
	int count = gObjectList.size();
	myList<BaseObject*>::iterator lastIter = --gObjectList.end();

	while (count > 1)
	{
		int max = 0;
		BaseObject* obj = nullptr;
		myList<BaseObject*>::iterator maxIter = gObjectList.begin();

		for (auto iter = gObjectList.begin(); iter != gObjectList.end(); iter++)
		{
			if ((*iter)->GetCurY() > max)
			{
				max = (*iter)->GetCurY();
				maxIter = iter;
			}
		}

		gObjectList.swapNode(maxIter, lastIter);
		--lastIter;
		count--;
	}
}
