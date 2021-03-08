#pragma once
#include "SceneTitle.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>

SceneTitle::SceneTitle()
	: mPrevTime(timeGetTime())
	, mbMessageOn(true)
	, mCSVReader(nullptr)
{
	LoadCSVFile();
}

void SceneTitle::GetKeyChange()
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

void SceneTitle::Update()
{
	int curTime = timeGetTime();

	if (curTime - mPrevTime >= 1000)
	{
		mbMessageOn = !mbMessageOn;
		mPrevTime = curTime;
	}
}

void SceneTitle::Render()
{
	const char* pBuf = mCSVReader->GetRowAddress(1);

	for (int i = 1; i <= mCSVReader->GetRow(); ++i)
	{
		RenderCSVFileData(i, &pBuf);
	}

	if (mbMessageOn == true)
	{
		char start_str[] = "Press enter to start game ...";
		sprintf_s(szScreenBuffer[18] + 25, dfSCREEN_WIDTH - 25, start_str);
		szScreenBuffer[18][strlen(start_str) + 25] = ' ';
	}
}

void SceneTitle::LoadCSVFile()
{
	char* pName = SceneManager::mFileNameArray[(int)SceneType::SCENE_TITLE];

	mCSVReader = new CSVFile(pName);
}
