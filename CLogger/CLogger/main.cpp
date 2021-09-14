#include "CLogger.h"
#include <process.h>
#include <wchar.h>

#define THREAD_SIZE (20)

using namespace std;

bool g_exit = false;

unsigned int WINAPI WorkerThread(LPVOID lpParam);

int main()
{
	CLogger::Initialize();
	CLogger::SetDirectory(L"../Logs");

	HANDLE hThreads[THREAD_SIZE];
	UINT32 seeds[THREAD_SIZE];

	for (int i = 1; i <= THREAD_SIZE; ++i)
	{
		seeds[i] = i * 1000;
	}

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, &seeds[i], 0, nullptr);
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

	for (int i = 0; i < THREAD_SIZE; ++i)
	{
		CloseHandle(hThreads[i]);
	}

	return 0;
}

unsigned int __stdcall WorkerThread(LPVOID lpParam)
{
	UINT32* seed = (UINT32*)lpParam;
	srand(*seed);

	while (!g_exit)
	{
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Test Log [%d] [%d]", rand(), rand());

		Sleep(0);

		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Test Log [%d] [%d]", rand(), rand());

		Sleep(0);

		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Test Log [%d] [%d]", rand(), rand());
	}

	return 0;
}
