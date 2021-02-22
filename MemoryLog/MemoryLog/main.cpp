#pragma once
#include <time.h>
#include "myNew.h"
#include "operatorNewOverload.h"
#include <memory.h>
#define MAX_SIZE (100)

myNew gMemoryPool[MAX_SIZE];

class CTest
{
public:
	~CTest()
	{
	}
private:
	int mX = 1;
};

int main()
{
	memset(gMemoryPool, 0, sizeof(gMemoryPool));

	int* test = new int[4];
	char* test2 = new char[4];
	CTest* test3 = new CTest[4];

	delete ((char*)test2 + 1);
	delete test2;
	delete test3;
	delete[] test3;

	return 0;
}