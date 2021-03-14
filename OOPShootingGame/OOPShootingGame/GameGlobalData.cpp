#pragma once
#include "GameGlobalData.h"
#include "SceneType.h"
#include "TxtReader.h"
#include "CSVReader.h"
#include <memory.h>
#include <stdlib.h>

GameGlobalData* GameGlobalData::mObject = nullptr;

GameGlobalData::GameGlobalData()
	: mNextSceneType(SceneType::SCENE_TITLE)
{
}

GameGlobalData::~GameGlobalData()
{
	for (int i = 0; i < mProcessFileListSize; ++i)
	{
		delete[] mProcessFileNameArray[i];
	}

	delete[] mProcessFileNameArray;

	for (int i = 0; i < mStageFileListSize; ++i)
	{
		delete[] mStageFileNameArray[i];
	}

	delete[] mStageFileNameArray;

	if (mObjectStats != nullptr)
		delete[] mObjectStats;
}

bool GameGlobalData::GetChangeScene()
{
	return mbChangeScene;
}

void GameGlobalData::SetChangeScene(bool flag)
{
	mbChangeScene = flag;
}

bool GameGlobalData::GetExit()
{
	return mbExit;
}

void GameGlobalData::SetExit(bool exit)
{
	mbExit = exit;
}

SceneType GameGlobalData::GetNextSceneType()
{
	return mNextSceneType;
}

void GameGlobalData::SetNextSceneType(SceneType type)
{
	mNextSceneType = type;
}

char** GameGlobalData::GetProcessFileNameArray()
{
	return mProcessFileNameArray;
}

char** GameGlobalData::GetStageFileNameArray()
{
	return mStageFileNameArray;
}

int GameGlobalData::GetCurrentStage()
{
	return mCurrentStage;
}

void GameGlobalData::SetCurrentStage(int stage)
{
	mCurrentStage = stage;
}

bool GameGlobalData::GetNextStage()
{
	return mbNextStage;
}

ObjectStat* GameGlobalData::GetObjectStats()
{
	return mObjectStats;
}

void GameGlobalData::Destroy()
{
	if (mObject != nullptr)
		delete mObject;
}

GameGlobalData* GameGlobalData::GetInstance()
{
	if (mObject == nullptr)
	{
		mObject = new GameGlobalData();
	}

	return mObject;
}

void GameGlobalData::LoadFileNameList()
{
	TxtReader txtReader(mFileList);

	char* pBegin = txtReader.getBuffer();
	char* pEnd = pBegin;
	txtReader.ReadNextParagraph(&pBegin, &pEnd, &mProcessFileListSize, &mProcessFileNameArray);

	pBegin = pEnd;
	txtReader.ReadNextParagraph(&pBegin, &pEnd, &mStageFileListSize, &mStageFileNameArray);
}

void GameGlobalData::Init()
{
	LoadFileNameList();
	LoadCSVFile("unit_stat.csv");
}

void GameGlobalData::LoadCSVFile(const char* fileName)
{
	CSVFile csvFile(fileName);

	int mObjectSize = csvFile.GetRow();
	int mObjectColSize = csvFile.GetCol();

	mObjectStats = new ObjectStat[mObjectSize];

	const char* pBegin = csvFile.GetRowAddress(1);

	for (int i = 0; i < mObjectSize; ++i)
	{
		char buffer[64] = { 0, };

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			memcpy(mObjectStats[i].name, buffer, sizeof(buffer));
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].hp = atoi(buffer);
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].damage = atoi(buffer);
		}

		if (csvFile.CopyToNextComma(buffer, &pBegin) == true)
		{
			mObjectStats[i].image = *buffer;
		}
	}
}