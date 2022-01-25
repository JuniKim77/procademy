#include <Windows.h>
#include <stdio.h>
#include "CProfilerClock.h"
#include <process.h>
#include <processthreadsapi.h>
#include <wchar.h>

int main()
{
	int a = 0;

	procademy::CProfilerClock::Begin(L"test");

	for (int i = 0; i < 10000; ++i)
	{
		a++;
	}

	procademy::CProfilerClock::End(L"test");

	procademy::CProfilerClock::Print();

	return 0;
}