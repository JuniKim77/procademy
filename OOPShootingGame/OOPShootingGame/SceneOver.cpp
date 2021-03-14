#pragma once
#include "SceneOver.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "SceneType.h"
#include "GameGlobalData.h"

SceneOver::SceneOver()
	: SceneProcess(SceneType::SCENE_OVER, "Press enter to restart game ...")
{
	LoadCSVFile();
}

void SceneOver::GetKeyChange()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) // Enter key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_PLAY);
		GameGlobalData::GetInstance()->SetChangeScene(1);
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		GameGlobalData::GetInstance()->SetChangeScene(true);
		GameGlobalData::GetInstance()->SetNextSceneType(SceneType::SCENE_END);
	}
}
