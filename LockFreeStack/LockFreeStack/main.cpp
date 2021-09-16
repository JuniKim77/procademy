#include "LockFreeStack.h"
#include <process.h>
#include <wchar.h>
#include "CLogger.h"
#include "CCrashDump.h"

#define THREAD_SIZE (10)
#define MAX_ALLOC (100000)
#define THREAD_ALLOC (10000)

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI MonitorThread(LPVOID lpParam);

CLFStack g_st;

long PushTPS = 0;
long PopTPS = 0;

int main()
{
	HANDLE hThreads[THREAD_SIZE + 1];

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, nullptr, 0, nullptr);

	for (int i = 1; i <= THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	}

	WORD ControlKey;

	while (1)
	{
		ControlKey = _getwch();
		if (ControlKey == L'q' || ControlKey == L'Q')
		{
			//------------------------------------------------
			// 종료처리
			//------------------------------------------------
			g_exit = true;
			break;
		}
	}

	DWORD retval = WaitForMultipleObjects(THREAD_SIZE + 1, hThreads, TRUE, INFINITE);

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

	for (int i = 0; i <= THREAD_SIZE; ++i)
	{
		CloseHandle(hThreads[i]);
	}

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	while (!g_exit)
	{
		for (int i = 0; i < THREAD_ALLOC; ++i)
		{
			g_st.Push(i);
			InterlockedIncrement((long*)&PushTPS);
		}

		Sleep(0);

		for (int i = 0; i < 10000; ++i)
		{
			ULONG64 num;
			if (g_st.Pop(&num) == false)
			{
				CRASH();
			}
			InterlockedIncrement((long*)&PopTPS);
		}
	}

	return 0;
}

unsigned int __stdcall MonitorThread(LPVOID lpParam)
{
	while (1)
	{
		long push = PushTPS;
		long pop = PopTPS;

		PushTPS = 0;
		PopTPS = 0;

		wprintf(L"=====================================================================\n");
		wprintf(L"                        LockFreeStack Testing...                        \n");
		wprintf(L"=====================================================================\n\n");

		wprintf(L"---------------------------------------------------------------------\n\n");
		wprintf(L"Pop TPS			: %ld\n", pop);
		wprintf(L"Push  TPS		: %ld\n", push);
		wprintf(L"Stack Size		: %ld\n", g_st.GetSize());
		wprintf(L"---------------------------------------------------------------------\n\n\n");
		if (g_st.GetSize() > MAX_ALLOC)
		{
			CRASH();
		}

		Sleep(999);
	}

	return 0;
}
