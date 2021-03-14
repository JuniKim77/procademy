#pragma once
#include "SceneManager.h"
#include "operatorNewOverload.h"
#include "SceneTitle.h"
#include "SceneEnd.h"
#include "SceneOver.h"
#include "SceneVictory.h"
#include "SceneType.h"
#include "ScenePlay.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "TxtReader.h"
#include "ObjectManager.h"

SceneManager* SceneManager::mManager = nullptr;
bool SceneManager::mbChangeScene = true;
bool SceneManager::mbNextStage = false;
bool SceneManager::mbExit = false;
SceneType SceneManager::mNextSceneType = SceneType::SCENE_TITLE;
char** SceneManager::mProcessFileNameArray = nullptr;
char** SceneManager::mStageFileNameArray = nullptr;
int SceneManager::mCurrentStage = 1;

SceneManager::SceneManager()
	: mpScene(nullptr)
{
	Init();
	LoadScene();
}

void SceneManager::LoadFileNameList()
{
	TxtReader txtReader(mFileList);

	char* pBegin = txtReader.getBuffer();
	char* pEnd = pBegin;
	txtReader.ReadNextParagraph(&pBegin, &pEnd, &mProcessFileListSize, &mProcessFileNameArray);

	pBegin = pEnd;
	txtReader.ReadNextParagraph(&pBegin, &pEnd, &mStageFileListSize, &mStageFileNameArray);
}

SceneManager::~SceneManager()
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

	if (mpScene != nullptr)
		delete mpScene;
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
		delete mpScene;
		LoadScene();
	}
	mpScene->GetKeyChange();
	mpScene->Update();
	mpScene->Render();

	if (mbNextStage == true)
	{
		++mCurrentStage;
		mbNextStage = false;

		if (mCurrentStage > mStageFileListSize)
		{
			mbChangeScene = true;
			mNextSceneType = SceneType::SCENE_VICTORY;
			ObjectManager::GetInstance()->ClearObjects();
		}
		else
		{
			mpScene->LoadCSVFile();
		}
		
	}
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
		mpScene = new ScenePlay();
		mNextSceneType = SceneType::SCENE_PLAY;
		mbChangeScene = false;
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
		ObjectManager::GetInstance()->ClearObjects();
		mpScene = new SceneEnd();
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

void SceneManager::Destroy()
{
	if (mManager != nullptr)
		delete mManager;
}
