#pragma once
#include "operatorNewOverload.h"

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
	int* test = new int[4];
	char* test2 = new char[4];
	CTest* test3 = new CTest[4];

	delete ((char*)test2 + 1);
	delete test2;
	delete test3;
	delete test3;
	delete[] test3;

	return 0;
}