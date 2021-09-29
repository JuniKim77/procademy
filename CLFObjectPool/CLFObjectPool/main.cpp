#define TEMPLE
#define VERSION_A
//#define VERSION_B

#include "CLFObjectPool.h"
#include <iostream>
#include <process.h>
#include "CCrashDump.h"
#include "CDebugger.h"
#include "TC_LFObjectPool.h"

#define THREAD_SIZE (3)
#define MAX_ALLOC (3000)
#define THREAD_ALLOC (1000)

struct st_DATA
{
	ULONG64 data;
	ULONG64 count;
};

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI	MonitorThread(LPVOID lpParam);
void Init();

#ifdef TEMPLE
procademy::TC_LFObjectPool<st_DATA> g_pool;
#else
procademy::CLFObjectPool g_pool;
#endif



long lInTPS = 0;
long lOutTPS = 0;

long lInCounter = 0;
long lOutCounter = 0;

int main()
{
#ifdef VERSION_A
	Init();
#endif

	procademy::CCrashDump::SetHandlerDump();
	CDebugger::Initialize();
	CDebugger::SetDirectory(L"./Debugs");

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

	CDebugger::Destroy();

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	st_DATA* pDataArray[THREAD_ALLOC];

#ifdef VERSION_A
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
			if (pDataArray[i]->data != 0x0000000055555555 ||
				pDataArray[i]->count != 0)
			{
				CRASH();
			}
		}
		// Increment
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			InterlockedIncrement64((LONG64*)&pDataArray[i]->data);
			InterlockedIncrement64((LONG64*)&pDataArray[i]->count);
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
			InterlockedDecrement64((LONG64*)&pDataArray[i]->data);
			InterlockedDecrement64((LONG64*)&pDataArray[i]->count);
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
			g_pool.Free(pDataArray[i]);
			InterlockedIncrement((long*)&lInCounter);
		}
		// Context Switching
		Sleep(0);
	}
#else
	while (!g_exit)
	{
		for (int i = 0; i < THREAD_ALLOC; ++i)
		{
			pDataArray[i] = g_pool.Alloc();
			InterlockedIncrement((long*)&lOutCounter);
		}

		Sleep(0);

		for (int i = 0; i < THREAD_ALLOC; ++i)
		{
			if (g_pool.Free(pDataArray[i]) == false)
			{
				CRASH();
			}
			InterlockedIncrement((long*)&lInCounter);
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
		lInTPS = lInCounter;
		lOutTPS = lOutCounter;

		lInCounter = 0;
		lOutCounter = 0;

		wprintf(L"---------------------------------------------------------------------\n\n");
		wprintf(L"[Alloc TPS	%ld]\n", lOutTPS);
		wprintf(L"[Free  TPS	%ld]\n", lInTPS);
		wprintf(L"[Alloc Count	%ld]\n", g_pool.GetSize());
		wprintf(L"[Malloc Count	%ld]\n", g_pool.GetMallocCount());
		wprintf(L"[Pool Capa	%ld]\n", g_pool.GetCapacity());
		wprintf(L"---------------------------------------------------------------------\n\n\n");
		/*if (g_pool.GetSize() > MAX_ALLOC)
		{
			CRASH();
		}*/

		Sleep(999);
	}

	return 0;
}

void Init()
{
	st_DATA* pDataArray[MAX_ALLOC];

	for (int i = 0; i < MAX_ALLOC; i++)
	{
		pDataArray[i] = g_pool.Alloc();
		pDataArray[i]->data = 0x0000000055555555;
		pDataArray[i]->count = 0;
	}

	// 2. 스택에 넣음
	for (int i = 0; i < MAX_ALLOC; i++)
	{
		g_pool.Free(pDataArray[i]);
	}
}
