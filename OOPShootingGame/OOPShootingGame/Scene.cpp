#pragma once
#include "Scene.h"

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
