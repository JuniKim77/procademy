#define TEMPLE
#define VERSION_A

#include "TC_LFStack.h"
#include <process.h>
#include <wchar.h>
#include "CLogger.h"
#include "CCrashDump.h"
 
#define THREAD_SIZE (3)
#define MAX_ALLOC (3)
#define THREAD_ALLOC (1)

struct st_DATA
{
	LONG64 data = 0x0000000055555555;
	LONG64 count = 0;
};

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI MonitorThread(LPVOID lpParam);

void Init();

alignas(64) TC_LFStack<st_DATA*> g_st;
DWORD g_MultiProfiler;

long PushTPS = 0;
long PopTPS = 0;

int main()
{
	g_MultiProfiler = TlsAlloc();

	Init();

	procademy::CCrashDump::SetHandlerDump();

	HANDLE hThreads[THREAD_SIZE + 1];

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	}

	hThreads[THREAD_SIZE] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, nullptr, 0, nullptr);

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
	st_DATA* pDataArray[THREAD_ALLOC];

	TlsSetValue(g_MultiProfiler, lpParam);

#ifdef VERSION_A
	while (!g_exit)
	{
		// Alloc
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			bool ret = g_st.Pop(&pDataArray[i]);
			if (ret == false)
			{
				int test = 0;
			}
			InterlockedIncrement((long*)&PopTPS);
		}
		// Check Init Data Value
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (pDataArray[i]->data != 0x0000000055555555 ||
				pDataArray[i]->count != 0)
			{
				CRASH();
			}
		}
		// Increment
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			InterlockedIncrement64(&pDataArray[i]->data);
			InterlockedIncrement64(&pDataArray[i]->count);
		}
		// Context Switching
		//Sleep(0);

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (pDataArray[i]->data != 0x0000000055555556 ||
				pDataArray[i]->count != 1)
			{
				CRASH();
			}
		}
		// Decrement
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			InterlockedDecrement64(&pDataArray[i]->data);
			InterlockedDecrement64(&pDataArray[i]->count);
		}
		// Context Switching
		Sleep(0);
		// Check Init Data Value
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (pDataArray[i]->data != 0x0000000055555555 ||
				pDataArray[i]->count != 0)
			{
				CRASH();
			}
		}

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			g_st.Push(pDataArray[i]);
			InterlockedIncrement((long*)&PushTPS);
		}
		// Context Switching
		Sleep(0);
	}
#else
	while (!g_exit)
	{
		for (int i = 0; i < THREAD_ALLOC; ++i)
		{
			bool ret = g_st.Pop(&pDataArray[i], &pDataInfo[i]);
			InterlockedIncrement((long*)&PushTPS);
		}

		Sleep(0);

		for (int i = 0; i < THREAD_ALLOC; ++i)
		{
			g_st.Push(pDataArray[i]);
			InterlockedIncrement((long*)&PopTPS);
		}

		Sleep(0);
	}
#endif

	return 0;
}

unsigned int __stdcall MonitorThread(LPVOID lpParam)
{
	while (!g_exit)
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
		wprintf(L"Malloc Size		: %ld\n", g_st.GetPoolCapacity());
		wprintf(L"---------------------------------------------------------------------\n\n\n");
		if (g_st.GetSize() > MAX_ALLOC)
		{
			CRASH();
		}

		Sleep(999);
	}

	return 0;
}

void Init()
{
	st_DATA* pDataArray[MAX_ALLOC];

	for (int i = 0; i < MAX_ALLOC; i++)
	{
		pDataArray[i] = new st_DATA;
		pDataArray[i]->data = 0x0000000055555555;
		pDataArray[i]->count = 0;
	}

	// 2. 스택에 넣음
	for (int i = 0; i < MAX_ALLOC; i++)
	{
		g_st.Push(pDataArray[i]);
	}

	g_st.linkCheck(MAX_ALLOC);
}
