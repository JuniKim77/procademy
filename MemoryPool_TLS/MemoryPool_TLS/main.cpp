#include "ObjectPool_TLS.h"
#include "CProfiler.h"
#include <process.h>
#include <wchar.h>
#include "TC_LFQueue.h"

#define dfTHREAD_SIZE (3)
#define dfTEST_SIZE (10000)
#define THREAD_ALLOC (2)
#define MAX_ALLOC (6)

class CTest
{
public:
	//char test[1024];
	LONG64 data;
	LONG64 count;
};

unsigned int WINAPI MonitorThread(LPVOID lpParam);
unsigned int WINAPI WorkerThread(LPVOID lpParam);
void Init();

void TLS_ALLOC_PROC();
void TLS_FREE_PROC();
void NEW_DELETE_ALLOC_PROC();
void NEW_DELETE_FREE_PROC();

bool g_exit = false;

alignas(64) long AllocTPS;
alignas(64) long FreeTPS;

__declspec(thread) CTest* arr1[dfTEST_SIZE];
__declspec(thread) CTest* arr2[dfTEST_SIZE];
procademy::ObjectPool_TLS<CTest> g_pool_tls;
procademy::TC_LFQueue<CTest*> g_q;

int main()
{
	procademy::CCrashDump::SetHandlerDump();
	//CProfiler::InitProfiler(dfTHREAD_SIZE);

	Init();

	HANDLE hThreads[dfTHREAD_SIZE + 1];

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, nullptr, 0, nullptr);

	for (int i = 1; i <= dfTHREAD_SIZE; ++i)
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

	DWORD retval = WaitForMultipleObjects(dfTHREAD_SIZE + 1, hThreads, TRUE, INFINITE);

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

	//CProfiler::Print();
	//CProfiler::DestroyProfiler();

	return 0;
}

unsigned int __stdcall MonitorThread(LPVOID lpParam)
{
	while (!g_exit)
	{
		long alloc = AllocTPS;
		long _free = FreeTPS;

		AllocTPS = 0;
		FreeTPS = 0;

		wprintf(L"---------------------------------------------------------------------\n\n");
		wprintf(L"[Alloc TPS		: %ld\n", alloc);
		wprintf(L"[Free  TPS		: %ld\n", _free);
		wprintf(L"[Pool Size		: %ld\n", g_pool_tls.GetSize());
		wprintf(L"[Pool Capa		: %ld\n", g_pool_tls.GetCapacity());
		wprintf(L"---------------------------------------------------------------------\n\n\n");

		Sleep(999);
	}

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	CTest* pDataArray[THREAD_ALLOC];

	while (!g_exit)
	{
		// Alloc
		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			pDataArray[i] = g_pool_tls.Alloc();

			InterlockedIncrement((long*)&AllocTPS);
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

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			g_q.Enqueue(pDataArray[i]);
		}

		Sleep(0);

		for (int i = 0; i < THREAD_ALLOC; i++)
		{
			g_q.Dequeue(&pDataArray[i]);
		}

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
			g_pool_tls.Free(pDataArray[i]);
			InterlockedIncrement((long*)&FreeTPS);
		}
		// Context Switching
		Sleep(0);
	}

	return 0;
}

void Init()
{
	CTest* pDataArray[200];

	for (DWORD i = 0; i < 200; ++i)
	{
		pDataArray[i] = g_pool_tls.Alloc();
		pDataArray[i]->data = 0x0000000055555555;
		pDataArray[i]->count = 0;
	}

	for (DWORD i = 0; i < 200; ++i)
	{
		g_pool_tls.Free(pDataArray[i]);
	}
}

void TLS_ALLOC_PROC()
{
	for (DWORD i = 0; i < dfTEST_SIZE; ++i)
	{
		arr1[i] = g_pool_tls.Alloc();
	}
}

void TLS_FREE_PROC()
{
	for (DWORD i = 0; i < dfTEST_SIZE; ++i)
	{
		g_pool_tls.Free(arr1[i]);
	}
}

void NEW_DELETE_ALLOC_PROC()
{
	for (DWORD i = 0; i < dfTEST_SIZE; ++i)
	{
#ifdef TEST_A

		arr2[i] = new int;
#else
		arr2[i] = new CTest;
#endif // TEST_A

	}
}

void NEW_DELETE_FREE_PROC()
{
	for (DWORD i = 0; i < dfTEST_SIZE; ++i)
	{
		delete arr2[i];
	}
}

