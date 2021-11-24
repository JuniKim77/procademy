#include <Windows.h>
#include <stdio.h>
#include "CProfiler.h"
#include <process.h>
#include <processthreadsapi.h>
#include <wchar.h>

#define dfTHREAD_NUM (4)

unsigned int WINAPI workerThread(LPVOID arg);

void test(int count)
{
	if (count == 0)
		return;

	test(count - 1);
}

void test1();
void test2();

HANDLE g_hExitThreadEvent;

int main()
{
	CProfiler::InitProfiler(dfTHREAD_NUM);
	g_hExitThreadEvent = CreateEvent(NULL, false, false, nullptr);

	HANDLE hThreads[dfTHREAD_NUM];

	for (int i = 0; i < dfTHREAD_NUM; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, nullptr, 0, nullptr);
	}

	WCHAR ControlKey;

	while (1)
	{
		ControlKey = _getwch();
		if (ControlKey == L'q' || ControlKey == L'Q')
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}

		if (ControlKey == L'p' || ControlKey == L'P')
		{
			CProfiler::Print();
		}
	}

	//------------------------------------------------
	// 스레드 종료 대기
	//------------------------------------------------
	DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, hThreads, TRUE, INFINITE);

	switch (retval)
	{
	case WAIT_FAILED:
		wprintf_s(L"Main Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		wprintf_s(L"Main Thread Timeout Error\n");
		break;
	case WAIT_OBJECT_0:
		wprintf_s(L"None Error\n");
		break;
	default:
		break;
	}

	return 0;
}

unsigned int __stdcall workerThread(LPVOID arg)
{
	DWORD dwError;
	LARGE_INTEGER begin;
	LARGE_INTEGER end;

	while (1)
	{
		dwError = WaitForSingleObject(g_hExitThreadEvent, 10);
		if (dwError != WAIT_TIMEOUT)
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}

		QueryPerformanceCounter(&begin);

		test1();

		test2();

		QueryPerformanceCounter(&end);

		CProfiler::SetRecord(L"Test3", end.QuadPart - begin.QuadPart);
	}

	return 0;
}

void test1()
{
	for (int i = 0; i < 100; ++i)
	{
		CProfiler::Begin(L"Test1");
		test(50);
		CProfiler::End(L"Test1");
	}
}

void test2()
{
	for (int i = 0; i < 100; ++i)
	{
		CProfiler::Begin(L"Test2");
		test(100);
		CProfiler::End(L"Test2");
	}
}

