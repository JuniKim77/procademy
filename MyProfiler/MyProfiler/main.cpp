#include <Windows.h>
#include <stdio.h>
#include "MyProfiler.h"

int main()
{
	ProfileReset();
	LARGE_INTEGER Start;
	LARGE_INTEGER End;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);	// 1���� �����ֱ�

	for (int i = 0; i < 10; ++i)
	{
		PRO_BEGIN(L"Func1");

		QueryPerformanceCounter(&Start);
		Sleep(200);
		QueryPerformanceCounter(&End);

		PRO_END(L"Func1");
	}

	ProfileDataOutText(TEXT("Profile"));

	return 0;
}