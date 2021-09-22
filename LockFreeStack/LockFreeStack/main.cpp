#define TEMPLE
#define VERSION_A

#include "TC_LFStack.h"
#include <process.h>
#include <wchar.h>
#include "CLogger.h"
#include "CCrashDump.h"
#include "CDebugger.h"
 
#define THREAD_SIZE (4)
#define MAX_ALLOC (40000)
#define THREAD_ALLOC (10000)

struct st_DATA
{
	LONG64 data;
	LONG64 count;
};

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI MonitorThread(LPVOID lpParam);

void Init();

TC_LFStack<st_DATA*> g_st;

long PushTPS = 0;
long PopTPS = 0;
DWORD g_records;
DWORD g_index;

int main()
{
	g_records = TlsAlloc();
	g_index = TlsAlloc();


	Init();


	procademy::CCrashDump::SetHandlerDump();

	//CDebugger::Initialize();
	//CDebugger::SetDirectory(L"./");

	HANDLE hThreads[THREAD_SIZE + 1];
	int args[THREAD_SIZE];

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		args[i] = i * 1000;
	}

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, nullptr, 0, nullptr);

	for (int i = 1; i <= THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, &args[i - 1], 0, nullptr);
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
	st_DEBUG* record = new st_DEBUG[USHRT_MAX];
	TlsSetValue(g_records, record);
	USHORT* index = new USHORT;
	*index = 0;
	TlsSetValue(g_index, index);

	st_DATA* pDataArray[THREAD_ALLOC];
	st_DEBUG* pDataInfo[THREAD_ALLOC];

#ifdef VERSION_A
	while (!g_exit)
	{
		// Alloc
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			bool ret = g_st.Pop(&pDataArray[i], &pDataInfo[i]);
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
		wprintf(L"Malloc Size		: %ld\n", g_st.GetMallocCount());
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
	st_DEBUG* record = new st_DEBUG[USHRT_MAX];
	TlsSetValue(g_records, record);
	USHORT* index = new USHORT;
	*index = 0;
	TlsSetValue(g_index, index);

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
