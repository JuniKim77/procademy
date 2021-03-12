#pragma once
#include "SceneOver.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "SceneType.h"

SceneOver::SceneOver()
	: SceneProcess(SceneType::SCENE_OVER, "Press enter to restart game ...")
{
	LoadCSVFile();
}

void SceneOver::GetKeyChange()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) // Enter key
	{
		SceneManager::mbChangeScene = true;
		SceneManager::mNextSceneType = SceneType::SCENE_PLAY;
		SceneManager::mCurrentStage = 1;
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		SceneManager::mbChangeScene = true;
		SceneManager::mNextSceneType = SceneType::SCENE_END;
	}
}
