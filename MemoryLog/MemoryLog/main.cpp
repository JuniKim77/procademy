#pragma once
#include <time.h>
#include "myNew.h"
#include "operatorNewOverload.h"
#include <memory.h>
#define MAX_SIZE (100)

myNew gMemoryPool[MAX_SIZE];

int main()
{
	memset(gMemoryPool, 0, sizeof(gMemoryPool));

	int* test = new int[4];
	char* test2 = new char[4];

	delete test2;

	return 0;
}