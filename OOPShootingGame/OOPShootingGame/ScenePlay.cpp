#pragma once
#include "ScenePlay.h"
#include "operatorNewOverload.h"
#include "ObjectManager.h"
#include "ObjectPlayer.h"
#include "ObjectEnemy.h"
#include <Windows.h>
#include "SceneManager.h"

ScenePlay::ScenePlay()
	: Scene(SceneType::SCENE_PLAY)
{
	ObjectManager::GetInstance()->AddObject(new ObjectPlayer(40, 20));

	LoadCSVFile();
}

void ScenePlay::GetKeyChange()
{
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		SceneManager::mbChangeScene = true;
		SceneManager::mNextSceneType = SceneType::SCENE_END;
	}
}

void ScenePlay::Update()
{
	ObjectManager::GetInstance()->Update();
}

void ScenePlay::Render()
{
	ObjectManager::GetInstance()->Render();
}

void ScenePlay::LoadCSVFile()
{
	CSVFile csvFile(SceneManager::mStageFileNameArray[SceneManager::mCurrentStage - 1]);

	const char* pBegin = csvFile.GetRowAddress(1);
	const char* pEnd = pBegin;

	

}
