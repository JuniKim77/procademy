#include "CLFObjectPool.h"
#include <iostream>

using namespace std;

int main()
{
	procademy::CLFObjectPool pool2(10, true);

	ULONG64* t1[13];

	for (int i = 0; i < 13; ++i)
	{
		t1[i] = pool2.Alloc();
		*t1[i] = i + 1;
	}

	for (int i = 0; i < 13; ++i)
	{
		pool2.Free(t1[i]);
	}

	return 0;
}