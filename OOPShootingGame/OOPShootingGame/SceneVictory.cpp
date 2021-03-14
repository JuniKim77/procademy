#pragma once
#include "SceneVictory.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "GameGlobalData.h"

SceneVictory::SceneVictory()
	: SceneProcess(SceneType::SCENE_VICTORY, "Press enter to restart game ...")
{
	LoadCSVFile();
}

void SceneVictory::GetKeyChange()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) // Enter key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_PLAY);
		GameGlobalData::GetInstance()->SetCurrentStage(1);
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_END);
	}
}