#pragma once
#include "Scene.h"
#include "SceneType.h"

Scene::~Scene()
{
	if (mCSVReader != nullptr)
		delete mCSVReader;
}

Scene::Scene(SceneType type)
	: mSceneType(type)
{
}

void Scene::RenderCSVFileData(int line, const char** pBuf)
{
	int idx = 0;

	for (int i = 0; i < dfSCREEN_WIDTH; ++i)
	{
		if (**pBuf == '\n' || **pBuf == '\0')
			break;

		if (**pBuf != ',')
		{
			szScreenBuffer[line][i] = **pBuf;
			++(*pBuf);
		}
		++(*pBuf);
	}

	++(*pBuf);
}
