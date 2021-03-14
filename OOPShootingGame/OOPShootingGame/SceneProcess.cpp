#pragma once
#include "SceneProcess.h"
#include <Windows.h>
#include "SceneManager.h"
#include <stdio.h>
#include "CSVReader.h"
#include "GameGlobalData.h"

void SceneProcess::Update()
{
	int curTime = timeGetTime();

	if (curTime - mPrevTime >= 1000)
	{
		mbMessageOn = !mbMessageOn;
		mPrevTime = curTime;
	}
}

void SceneProcess::Render()
{
	const char* pBuf = mCSVReader->GetRowAddress(1);

	for (int i = 1; i <= mCSVReader->GetRow(); ++i)
	{
		RenderCSVFileData(i, &pBuf);
	}

	if (mbMessageOn == true)
	{
		sprintf_s(szScreenBuffer[18] + 25, dfSCREEN_WIDTH - 25, mProcessMessage);
		szScreenBuffer[18][strlen(mProcessMessage) + 25] = ' ';
	}
}

SceneProcess::SceneProcess(SceneType type, const char* message)
	: Scene(type)
	, mPrevTime(timeGetTime())
	, mbMessageOn(true)
	, mProcessMessage(message)
{
}

void SceneProcess::LoadCSVFile()
{
	mCSVReader = new CSVFile(GameGlobalData::GetInstance()->GetProcessFileNameArray()[(int)mSceneType]);
}
