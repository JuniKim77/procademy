#pragma once
#include "SceneManager.h"
#include "operatorNewOverload.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "SceneOver.h"
#include "SceneVictory.h"
#include "SceneType.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

SceneManager* SceneManager::mManager = nullptr;
bool SceneManager::mbChangeScene = true;
bool SceneManager::mbExit = false;
SceneType SceneManager::mNextSceneType = SceneType::SCENE_TITLE;
char** SceneManager::mProcessFileNameArray = nullptr;
char** SceneManager::mStageFileNameArray = nullptr;
int SceneManager::mCurrentStage = 1;

void SceneManager::GetNextLine(char** pBegin, char** pEnd, char* buffer)
{
	while (**pEnd != '\n' && **pEnd != '\0')
	{
		++(*pEnd);
	}

	memcpy(buffer, *pBegin, *pEnd - *pBegin);

	++(*pEnd);
}

SceneManager::SceneManager()
	: mpScene(nullptr)
{
	Init();
	LoadScene();
}

void SceneManager::LoadFileNameList()
{
	FILE* fin;
	char* file_memory;
	char buffer[FILE_NAME_SIZE] = { 0, };

	fopen_s(&fin, mFileList, "r");

	fseek(fin, 0, SEEK_END);
	int file_size = ftell(fin);

	file_memory = new char[file_size];

	memset(file_memory, 0, file_size);

	fseek(fin, 0, SEEK_SET);
	fread_s(file_memory, file_size, file_size, 1, fin);

	char* pBegin = file_memory;
	char* pEnd = file_memory;

	GetNextLine(&pBegin, &pEnd, buffer);
	mProcessFileListSize = atoi(buffer);

	mProcessFileNameArray = (char**)(new char* [mProcessFileListSize]);

	for (int i = 0; i < mProcessFileListSize; ++i)
	{
		pBegin = pEnd;
		GetNextLine(&pBegin, &pEnd, buffer);
		int size = pEnd - pBegin;
		mProcessFileNameArray[i] = new char[size];

		memcpy(mProcessFileNameArray[i], buffer, size);
		mProcessFileNameArray[i][size - 1] = '\0';
	}

	pBegin = pEnd;
	GetNextLine(&pBegin, &pEnd, buffer);
	mStageFileListSize = atoi(buffer);

	mStageFileNameArray = (char**)(new char* [mStageFileListSize]);

	for (int i = 0; i < mStageFileListSize; ++i)
	{
		pBegin = pEnd;
		GetNextLine(&pBegin, &pEnd, buffer);
		int size = pEnd - pBegin;
		mStageFileNameArray[i] = new char[size];

		memcpy(mStageFileNameArray[i], buffer, size);
		mStageFileNameArray[i][size - 1] = '\0';
	}
	
	delete[] file_memory;
	fclose(fin);
}

SceneManager::~SceneManager()
{
	if (mManager != nullptr)
		delete mManager;

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
}

SceneManager* SceneManager::GetInstance()
{
	if (mManager == nullptr)
	{
		mManager = new SceneManager();
	}
	return mManager;
}

void SceneManager::Run()
{
	if (mbChangeScene == true)
	{
		LoadScene();
	}
	mpScene->GetKeyChange();
	mpScene->Update();
	mpScene->Render();
}

void SceneManager::LoadScene()
{
	switch (mNextSceneType)
	{
	case SceneType::SCENE_TITLE:
		mpScene = new SceneTitle();
		mNextSceneType = SceneType::SCENE_PLAY;
		mbChangeScene = false;
		break;
	case SceneType::SCENE_PLAY:
		
		break;
	case SceneType::SCENE_VICTORY:
		mpScene = new SceneVictory();
		mNextSceneType = SceneType::SCENE_END;
		mbChangeScene = false;
		break;
	case SceneType::SCENE_OVER:
		mpScene = new SceneOver();
		mNextSceneType = SceneType::SCENE_END;
		mbChangeScene = false;
		break;
	case SceneType::SCENE_END:
		mpScene = new SceneEnd();
		mNextSceneType = SceneType::SCENE_END;
		mbChangeScene = false;
		break;
	default:
		break;
	}
}

void SceneManager::Init()
{
	LoadFileNameList();
}
