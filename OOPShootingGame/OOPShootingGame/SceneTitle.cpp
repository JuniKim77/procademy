#pragma once
#include "SceneTitle.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "SceneType.h"
#include "GameGlobalData.h"

SceneTitle::SceneTitle()
	: SceneProcess(SceneType::SCENE_TITLE, "Press enter to start game ...")
{
	LoadCSVFile();
}

void SceneTitle::GetKeyChange()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) // Enter key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_PLAY);
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_END);
	}
}