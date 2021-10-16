#define TEMPLE

#include "TC_LFQueue.h"
#include "CLogger.h"
#include "CLFQueue.h"
#include "CCrashDump.h"
#include "CProfiler.h"
#include <process.h>
#include <wchar.h>

#define THREAD_SIZE (3)
#define MAX_ALLOC (6)
#define THREAD_ALLOC (2)

extern USHORT g_debug_index;
extern st_DEBUG g_debugs[USHRT_MAX + 1];

struct st_DATA
{
	LONG64 data;
	LONG64 count;
};

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);
unsigned int WINAPI MonitorThread(LPVOID lpParam);
void ReqTextOut(CProfiler** profilers);
void Init();

TC_LFQueue<st_DATA*> g_q;

long PushTPS = 0;
long DequeueTPS = 0;
DWORD g_MultiProfiler;

int main()
{
	procademy::CCrashDump::SetHandlerDump();
	Init();

	HANDLE hThreads[THREAD_SIZE + 1];
	CProfiler* profilers[THREAD_SIZE] = { 0, };

	g_MultiProfiler = TlsAlloc();	

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		profilers[i] = new CProfiler(L"settings.csv");
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, profilers[i], 0, nullptr);
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

		if (ControlKey == L'p' || ControlKey == L'P')
		{
			ReqTextOut(profilers);
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
	((CProfiler*)lpParam)->SetThreadId();

	while (!g_exit)
	{
		// Alloc
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (g_exit)
			{
				return 0;
			}
			((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileBegin(L"DEQUEUE");
			bool ret = g_q.Dequeue(&pDataArray[i]);
			((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileEnd(L"DEQUEUE");
			if (ret == false)
			{
				CRASH();
			}
			InterlockedIncrement((long*)&DequeueTPS);
		}
		// Check Init Data Value
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (pDataArray[i]->data != 0x0000000055555555 ||
				pDataArray[i]->count != 0)
			{
				USHORT idx = finder_log();
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
				USHORT idx = finder_log();
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
				USHORT idx = finder_log();
				CRASH();
			}
		}

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			if (g_exit)
			{
				return 0;
			}
			((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileBegin(L"ENQUEUE");
			g_q.Enqueue(pDataArray[i]);
			((CProfiler*)TlsGetValue(g_MultiProfiler))->ProfileEnd(L"ENQUEUE");
			pDataArray[i] = nullptr;
			InterlockedIncrement((long*)&PushTPS);
		}
		// Context Switching
		Sleep(0);
	}

	return 0;
}

unsigned int __stdcall MonitorThread(LPVOID lpParam)
{
	while (!g_exit)
	{
		long push = PushTPS;
		long pop = DequeueTPS;

		PushTPS = 0;
		DequeueTPS = 0;

		wprintf(L"---------------------------------------------------------------------\n\n");
		wprintf(L"[Enqueue TPS		: %ld\n", pop);
		wprintf(L"[Dequeue  TPS		: %ld\n", push);
		wprintf(L"[Queue Size		: %ld\n", g_q.GetSize());
		wprintf(L"[Pool Capa		: %ld\n", g_q.GetPoolCapacity());
		wprintf(L"---------------------------------------------------------------------\n\n\n");
		if (g_q.GetSize() > MAX_ALLOC)
		{
			CRASH();
		}

		Sleep(999);
	}

	return 0;
}

void ReqTextOut(CProfiler** profilers)
{
	WCHAR fileName[FILE_NAME_MAX] = L"Profile";

	CProfiler::SetProfileFileName(fileName);

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		profilers[i]->ProfileDataOutText(fileName);
		profilers[i]->ProfileReset();
	}
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
		g_q.Enqueue(pDataArray[i]);
	}

	g_q.linkCheck(MAX_ALLOC);
}
