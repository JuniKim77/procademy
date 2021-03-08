#pragma once
#include "SceneManager.h"
#include "operatorNewOverload.h"
#include "SceneTitle.h"
#include "SceneType.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

SceneManager* SceneManager::mManager = nullptr;
bool SceneManager::mbChangeScene = true;
SceneType SceneManager::mNextSceneType = SceneType::SCENE_TITLE;
char** SceneManager::mFileNameArray = nullptr;

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
	mFileListSize = atoi(buffer);

	mFileNameArray = (char**)(new char* [mFileListSize]);

	for (int i = 0; i < mFileListSize; ++i)
	{
		pBegin = pEnd;
		GetNextLine(&pBegin, &pEnd, buffer);
		int size = pEnd - pBegin;
		mFileNameArray[i] = new char[size];
		

		memcpy(mFileNameArray[i], buffer, size);
		mFileNameArray[i][size - 1] = '\0';
	}
	
	delete[] file_memory;
	fclose(fin);
}

SceneManager::~SceneManager()
{
	if (mManager != nullptr)
		delete mManager;

	for (int i = 0; i < mFileListSize; ++i)
	{
		delete[] mFileNameArray[i];
	}
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
	mpScene->GetKeyChange();
	mpScene->Update();
	mpScene->Render();
}

void SceneManager::LoadScene()
{
	if (mbChangeScene == false)
		return;

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

		break;
	case SceneType::SCENE_OVER:

		break;
	case SceneType::SCENE_END:
		mpScene = new SceneTitle();
		mNextSceneType = SceneType::SCENE_PLAY;
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
