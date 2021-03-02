#include <Windows.h>
#include <stdio.h>
#include "MyProfiler.h"

void test(int count)
{
	if (count == 0)
		return;

	test(count - 1);
}

int main()
{
	ProfileReset();
	LARGE_INTEGER Start;
	LARGE_INTEGER End;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);	// 1초의 진동주기

	for (int i = 0; i < 100; ++i)
	{
		PRO_BEGIN(L"Test1");

		QueryPerformanceCounter(&Start);

		test(50);

		QueryPerformanceCounter(&End);

		PRO_END(L"Test1");
	}

	for (int i = 0; i < 100; ++i)
	{
		PRO_BEGIN(L"Test2");

		QueryPerformanceCounter(&Start);

		test(100);

		QueryPerformanceCounter(&End);

		PRO_END(L"Test2");
	}

	ProfileDataOutText(TEXT("Profile"));

	return 0;
}