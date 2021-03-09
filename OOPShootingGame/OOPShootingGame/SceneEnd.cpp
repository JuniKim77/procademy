#pragma once
#include "SceneEnd.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>

SceneEnd::SceneEnd()
	: Scene(SceneType::SCENE_END)
{
	LoadCSVFile();
}

void SceneEnd::GetKeyChange()
{
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001) // ESC key
	{
		SceneManager::mbExit = true;
	}
}

void SceneEnd::Update()
{
	SceneManager::mbExit = true;
}

void SceneEnd::Render()
{
	const char* pBuf = mCSVReader->GetRowAddress(1);

	for (int i = 1; i <= mCSVReader->GetRow(); ++i)
	{
		RenderCSVFileData(i, &pBuf);
	}
}

void SceneEnd::LoadCSVFile()
{
	mCSVReader = new CSVFile(SceneManager::mProcessFileNameArray[(int)mSceneType]);
}
