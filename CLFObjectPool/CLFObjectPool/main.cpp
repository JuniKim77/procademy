#include "CLFObjectPool.h"
#include <iostream>
#include <process.h>
#include "CCrashDump.h"

#define THREAD_SIZE (10)
#define MAX_ALLOC (100000)
#define THREAD_ALLOC (10000)

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
void Initialize();

procademy::CLFObjectPool g_pool(10, true);

long lInTPS = 0;
long lOutTPS = 0;
	 
long lInCounter = 0;
long lOutCounter = 0;

int main()
{
	Initialize();

	HANDLE hThreads[THREAD_SIZE];

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	}

	WORD ControlKey;

	while (1)
	{
		lInTPS = lInCounter;
		lOutTPS = lOutCounter;

		lInCounter = 0;
		lOutCounter = 0;

		wprintf(L"=====================================================================\n");
		wprintf(L"                        MemoryPool Testing...                        \n");
		wprintf(L"=====================================================================\n\n");

		wprintf(L"---------------------------------------------------------------------\n\n");
		wprintf(L"Alloc TPS		: %ld\n", lOutTPS);
		wprintf(L"Free  TPS		: %ld\n", lInTPS);
		wprintf(L"Alloc TPS		: %ld\n", g_pool.GetSize());
		wprintf(L"---------------------------------------------------------------------\n\n\n");
		if (g_pool.GetSize() > MAX_ALLOC)
		{
			CRASH();
		}

		Sleep(999);
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

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		CloseHandle(hThreads[i]);
	}

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	ULONG64* pDataArray[THREAD_ALLOC];

	while (!g_exit)
	{
		// Alloc
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			pDataArray[i] = g_pool.Alloc();
			InterlockedIncrement((long*)&lOutCounter);
		}
		// Check Init Data Value
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (*pDataArray[i] != 0x0000000055555555)
			{
				CRASH();
			}
		}
		// Increment
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			InterlockedIncrement64((LONG64*)pDataArray[i]);
		}
		// Context Switching
		Sleep(0);

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (*pDataArray[i] != 0x0000000055555556)
			{
				CRASH();
			}
		}
		// Decrement
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			InterlockedDecrement64((LONG64*)pDataArray[i]);
		}
		// Context Switching
		Sleep(0);
		// Check Init Data Value
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (*pDataArray[i] != 0x0000000055555555)
			{
				CRASH();
			}
		}

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			g_pool.Free(pDataArray[i]);
			InterlockedIncrement((long*)&lInCounter);
		}
	}

	return 0;
}

void Initialize()
{
	ULONG64** pDataArray = new ULONG64*[MAX_ALLOC];

	for (int i = 0; i < MAX_ALLOC; i++)
	{
		pDataArray[i] = g_pool.Alloc();
	}
		

	for (int i = 0; i < MAX_ALLOC; i++)
	{
		*pDataArray[i] = 0x0000000055555555;
	}

	for (int i = 0; i < MAX_ALLOC; i++)
	{
		g_pool.Free(pDataArray[i]);
	}

	delete[] pDataArray;
}
