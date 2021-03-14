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
#include "GameGlobalData.h"

SceneManager* SceneManager::mManager = nullptr;

SceneManager::SceneManager()
	: mpScene(nullptr)
{
	LoadScene();
}

SceneManager::~SceneManager()
{
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
	GameGlobalData* pGlobalData = GameGlobalData::GetInstance();

	if (pGlobalData->mbChangeScene == true)
	{
		delete mpScene;
		LoadScene();
	}
	mpScene->GetKeyChange();
	mpScene->Update();
	mpScene->Render();

	if (pGlobalData->mbNextStage == true)
	{
		++pGlobalData->mCurrentStage;
		pGlobalData->mbNextStage = false;

		if (pGlobalData->mCurrentStage > pGlobalData->mStageFileListSize)
		{
			pGlobalData->mbChangeScene = true;
			pGlobalData->mNextSceneType = SceneType::SCENE_VICTORY;
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
	GameGlobalData* pGlobalData = GameGlobalData::GetInstance();

	switch (pGlobalData->mNextSceneType)
	{
	case SceneType::SCENE_TITLE:
		mpScene = new SceneTitle();
		pGlobalData->mNextSceneType = SceneType::SCENE_PLAY;
		pGlobalData->mbChangeScene = false;
		break;
	case SceneType::SCENE_PLAY:
		mpScene = new ScenePlay();
		pGlobalData->mNextSceneType = SceneType::SCENE_PLAY;
		pGlobalData->mbChangeScene = false;
		break;
	case SceneType::SCENE_VICTORY:
		mpScene = new SceneVictory();
		pGlobalData->mNextSceneType = SceneType::SCENE_END;
		pGlobalData->mbChangeScene = false;
		break;
	case SceneType::SCENE_OVER:
		mpScene = new SceneOver();
		pGlobalData->mNextSceneType = SceneType::SCENE_END;
		pGlobalData->mbChangeScene = false;
		break;
	case SceneType::SCENE_END:
		ObjectManager::GetInstance()->ClearObjects();
		mpScene = new SceneEnd();
		pGlobalData->mbChangeScene = false;
		break;
	default:
		break;
	}
}

void SceneManager::Destroy()
{
	if (mManager != nullptr)
		delete mManager;
}
