#pragma once
#include "TwoStar.h"

bool TwoStar::Update()
{
	mX += 2;

	return mX < MAX_RANGE - 1;
}

void TwoStar::Render()
{
	for (int i = 0; i < mX; ++i)
	{
		printf(" ");
	}
	printf("**");
}