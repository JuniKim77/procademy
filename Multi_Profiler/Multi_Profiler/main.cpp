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

void ReqTextOut(CProfiler** profilers);

DWORD g_MultiProfiler;
HANDLE g_hExitThreadEvent;

int main()
{
	g_MultiProfiler = TlsAlloc();

	g_hExitThreadEvent = CreateEvent(NULL, false, false, nullptr);

	CProfiler* profilers[dfTHREAD_NUM] = { 0, };

	HANDLE hThreads[dfTHREAD_NUM];

	for (int i = 0; i < dfTHREAD_NUM; ++i)
	{
		profilers[i] = new CProfiler(L"settings.csv");
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, profilers[i], 0, nullptr);
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
			ReqTextOut(profilers);
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
	TlsSetValue(g_MultiProfiler, arg);
	((CProfiler*)arg)->SetThreadId();
	
	DWORD dwError;

	while (1)
	{
		dwError = WaitForSingleObject(g_hExitThreadEvent, 10);
		if (dwError != WAIT_TIMEOUT)
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}

		test1();

		test2();
	}

	return 0;
}

void test1()
{
	for (int i = 0; i < 100; ++i)
	{
		((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileBegin(L"Test1");

		test(50);

		((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileEnd(L"Test1");
	}
}

void test2()
{
	for (int i = 0; i < 100; ++i)
	{
		((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileBegin(L"Test2");

		test(100);

		((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileEnd(L"Test2");
	}
}

void ReqTextOut(CProfiler** profilers)
{
	WCHAR fileName[FILE_NAME_MAX] = L"Profile";

	CProfiler::SetProfileFileName(fileName);

	for (int i = 0; i < dfTHREAD_NUM; ++i)
	{
		profilers[i]->ProfileDataOutText(fileName);
		profilers[i]->ProfileReset();
	}
}
