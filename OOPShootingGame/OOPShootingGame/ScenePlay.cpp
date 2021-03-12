#pragma once
#include "ScenePlay.h"
#include "operatorNewOverload.h"
#include "ObjectBase.h"
#include "ObjectManager.h"
#include "ObjectPlayer.h"
#include "ObjectEnemy.h"
#include <Windows.h>
#include "SceneManager.h"
#include "CSVReader.h"

ScenePlay::ScenePlay()
	: Scene(SceneType::SCENE_PLAY)
{
	ObjectManager* pManager = ObjectManager::GetInstance();
	pManager->AddObject(new ObjectPlayer(40, 20));

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
	ObjectManager::GetInstance()->ClearNonePlayerObjects();

	CSVFile csvFile(SceneManager::mStageFileNameArray[SceneManager::mCurrentStage - 1]);

	const char* pBegin = csvFile.GetRowAddress(1);
	char buffer[64] = { 0, };

	int row = csvFile.GetRow();
	int col = csvFile.GetCol();
	
	for (int y = 0; y < row - 1; ++y)
	{
		for (int x = 0; x < col; ++x)
		{
			if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
			{
				int unitType = atoi(buffer);

				ObjectManager::GetInstance()->AddObject(new ObjectEnemy(x, y, unitType));
			}
		}
	}
}
