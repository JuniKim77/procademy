#pragma once
#include "OneStar.h"

bool OneStar::Update()
{
	mX += 1;

	return mX < MAX_RANGE;
}

void OneStar::Render()
{
	for (int i = 0; i < mX; ++i)
	{
		printf(" ");
	}
	printf("*");
}
