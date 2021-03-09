#pragma once
#include "SceneVictory.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>

SceneVictory::SceneVictory()
	: SceneProcess(SceneType::SCENE_VICTORY, "Press enter to restart game ...")
{
	LoadCSVFile();
}

void SceneVictory::GetKeyChange()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001) // Enter key
	{
		SceneManager::mbChangeScene = true;
		SceneManager::mNextSceneType = SceneType::SCENE_PLAY;
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		SceneManager::mbChangeScene = true;
		SceneManager::mNextSceneType = SceneType::SCENE_END;
	}
}