#include <stdio.h>
#include "LockFreeStack.h"
#include <process.h>
#include <wchar.h>
#include "CLogger.h"

#define THREAD_SIZE (20)

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);

CLFStack g_st;

int main()
{
	g_st.Push(100);
	g_st.Push(100);
	g_st.Push(100);
	g_st.Push(100);

	HANDLE hThreads[THREAD_SIZE];

	for (int i = 0; i < THREAD_SIZE; ++i)
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

	DWORD retval = WaitForMultipleObjects(THREAD_SIZE, hThreads, TRUE, INFINITE);

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

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	while (!g_exit)
	{
		int t = 0;
		for (int i = 0; i < 100000; ++i)
		{
			g_st.Push(i);
		}

		for (int i = 0; i < 100000; ++i)
		{
			int num;
			g_st.Pop(&num);
		}
	}

	return 0;
}
