#include "CSafeStack.h"
#include <iostream>

int main()
{
	procademy::CSafeStack<int> test;

	test.Push(1);
	test.Push(2);
	test.Push(3);

	for (int i = 0; i < 3; ++i)
	{
		int num = test.Top();
		test.Pop();

		wprintf_s(L"%d\n", num);
	}

	return 0;
}