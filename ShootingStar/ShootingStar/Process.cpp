#pragma once
#include <conio.h>
#include "Process.h"
#include <stdio.h>
#define MAX_SIZE (20)

BaseObject* respawns[MAX_SIZE] = { 0, };

enum
{
	ONE = 49,
	TWO,
	THREE
};

void KeyProcess()
{
	if (_kbhit())
	{
		char key = _getch();

		for (int i = 0; i < MAX_SIZE; ++i)
		{
			if (respawns[i] == nullptr)
			{
				switch (key)
				{
				case ONE:
					respawns[i] = new OneStar();
					break;
				case TWO:
					respawns[i] = new TwoStar();
					break;
				case THREE:
					respawns[i] = new ThreeStar();
					break;
				default:
					break;
				}
				break;
			}
		}
	}
}

void Update()
{
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		if (respawns[i] != nullptr)
		{
			if (!respawns[i]->Update())
			{
				delete respawns[i];

				respawns[i] = nullptr;
			}
		};
	}
}

void Render()
{
	for (int i = 0; i < MAX_SIZE; ++i)
	{
		if (respawns[i] != nullptr)
		{
			respawns[i]->Render();
		}
		printf("\n");
	}
}
