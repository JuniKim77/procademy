#pragma once
#include "ThreeStar.h"

bool ThreeStar::Update()
{
	mX += 3;

	return mX < MAX_RANGE - 2;
}

void ThreeStar::Render()
{
	for (int i = 0; i < mX; ++i)
	{
		printf(" ");
	}

	printf("***");
}
