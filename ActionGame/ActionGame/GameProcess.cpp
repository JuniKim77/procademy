#pragma once
#include "GameProcess.h"
#include "ScreenDib.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "SpriteDib.h"
#include "ESprite.h"

Process gGameState = PROCESS_GAME;

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
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_01, L"SpriteData\\xSpark_1.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_02, L"SpriteData\\xSpark_2.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_03, L"SpriteData\\xSpark_3.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eEFFECT_SPARK_04, L"SpriteData\\xSpark_4.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eGUAGE_HP, L"SpriteData\\HPGuage.bmp", 0, 0);
	gSpriteDib.LoadDibSprite(eSHADOW, L"SpriteData\\Shadow.bmp", 0, 0);

	// 플레이어 생성
	gPlayerObject = new PlayerObject;
	gPlayerObject->SetPosition(320, 240);
	gObjectList.push_back(gPlayerObject);

	// 테스트용 객체들
	BaseObject* pObject = new PlayerObject;
	pObject->SetPosition(100, 100);
	gObjectList.push_back(pObject);
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
	Update();
	Render(); // 백버퍼에 출력

	gScreenDib.Filp(gMainWindow); // 윈도에 출력
}

void KeyProcess()
{
}

void Update()
{
}

void Render()
{
}
