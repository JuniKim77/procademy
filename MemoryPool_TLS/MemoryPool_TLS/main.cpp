#include "ObjectPool_TLS.h"
#include "CProfiler.h"
#include <process.h>
#include <wchar.h>

#define dfTHREAD_SIZE (5)
#define dfTEST_SIZE (100000)
//#define TEST_A
#define CHUNK_SIZE (1000)

class CTest
{
private:
	char test[1024];
};

unsigned int WINAPI WorkerThread(LPVOID lpParam);

void TLS_ALLOC_PROC();
void TLS_FREE_PROC();
void NEW_DELETE_ALLOC_PROC();
void NEW_DELETE_FREE_PROC();

#ifdef TEST_A
__declspec(thread) int* arr1[dfTEST_SIZE];
__declspec(thread) int* arr2[dfTEST_SIZE];
procademy::ObjectPool_TLS<int> g_pool_tls;
#else
__declspec(thread) CTest* arr1[dfTEST_SIZE];
__declspec(thread) CTest* arr2[dfTEST_SIZE];
procademy::ObjectPool_TLS<CTest> g_pool_tls;
#endif // TEST_A

int main()
{
	procademy::CCrashDump::SetHandlerDump();
	CProfiler::InitProfiler(dfTHREAD_SIZE);

	HANDLE hThreads[dfTHREAD_SIZE];

	for (int i = 0; i < dfTHREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	}

	DWORD retval = WaitForMultipleObjects(dfTHREAD_SIZE, hThreads, TRUE, INFINITE);

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

	CProfiler::Print();
	CProfiler::DestroyProfiler();

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	int count = 100;

	TLS_ALLOC_PROC();
	TLS_FREE_PROC();
	NEW_DELETE_ALLOC_PROC();
	NEW_DELETE_FREE_PROC();

	while (count-- > 0)
	{
		CProfiler::Begin(L"TLS_ALLOC_PROC");
		TLS_ALLOC_PROC();
		CProfiler::End(L"TLS_ALLOC_PROC");

		CProfiler::Begin(L"TLS_FREE_PROC");
		TLS_FREE_PROC();
		CProfiler::End(L"TLS_FREE_PROC");

		CProfiler::Begin(L"NEW_ALLOC_PROC");
		NEW_DELETE_ALLOC_PROC();
		CProfiler::End(L"NEW_ALLOC_PROC");

		CProfiler::Begin(L"NEW_FREE_PROC");
		NEW_DELETE_FREE_PROC();
		CProfiler::End(L"NEW_FREE_PROC");
	}

	return 0;
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

