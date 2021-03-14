#pragma once
#include "SceneEnd.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "CSVReader.h"
#include "GameGlobalData.h"

SceneEnd::SceneEnd()
	: Scene(SceneType::SCENE_END)
{
	LoadCSVFile();
}

void SceneEnd::GetKeyChange()
{
}

void SceneEnd::Update()
{
	GameGlobalData::GetInstance()->SetExit(true);
}

void SceneEnd::Render()
{
	const char* pBuf = mCSVReader->GetRowAddress(1);

	for (int i = 1; i <= mCSVReader->GetRow(); ++i)
	{
		RenderCSVFileData(i, &pBuf);
	}
}

SceneEnd::~SceneEnd()
{
}

void SceneEnd::LoadCSVFile()
{
	mCSVReader = new CSVFile(GameGlobalData::GetInstance()->GetProcessFileNameArray()[(int)mSceneType]);
}
