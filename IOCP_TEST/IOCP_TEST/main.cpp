#include <Windows.h>
#include <process.h>
#include <conio.h>
#include "CProfiler.h"
#include "TextParser.h"
#include "CLogger.h"

int g_worker_thread;
int g_active_thread;
int g_job1_count;
int g_job2_count;
int g_sleep_count;
HANDLE g_iocp;
HANDLE* g_handles;

void LoadInitData(const WCHAR* file);
void Init();
void KeyProc();
static unsigned int WINAPI WorkerThread(LPVOID arg);

int main()
{
	LoadInitData(L"init.ini");

	Init();

	KeyProc();

	DWORD retval = WaitForMultipleObjects(g_worker_thread, g_handles, true, INFINITE);

	switch (retval)
	{
	case WAIT_FAILED:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"IOCP_TEST Thread handle Error\n");
		break;
	case WAIT_TIMEOUT:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"IOCP_TEST TimeOut Error\n");
		break;
	case WAIT_OBJECT_0:
		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"IOCP_TEST End\n");
		break;
	default:
		break;
	}

	CProfiler::DestroyProfiler();

	return 0;
}

void LoadInitData(const WCHAR* file)
{
	TextParser  tp;
	int         num;
	WCHAR       buffer[MAX_PARSER_LENGTH];

	tp.LoadFile(file);

	tp.GetValue(L"IOCP_WORKER_THREAD", &g_worker_thread);
	tp.GetValue(L"IOCP_ACTIVE_THREAD", &g_active_thread);
	tp.GetValue(L"JOB1_COUNT", &g_job1_count);
	tp.GetValue(L"JOB2_COUNT", &g_job2_count);
	tp.GetValue(L"SLEEP_COUNT", &g_sleep_count);
}

void Init()
{
	CProfiler::InitProfiler(g_worker_thread + 1);

	CLogger::SetDirectory(L"_log");

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"IOCP_TEST Begin\n");

	g_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, (DWORD)g_active_thread);

	g_handles = new HANDLE[g_worker_thread];

	for (int i = 0; i < g_worker_thread; ++i)
	{
		g_handles[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
	}
}

void KeyProc()
{
	while (1)
	{
		char ch = _getch();

		switch (ch)
		{
		case 'p':
			CProfiler::Print();
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"IOCP_TEST Print\n");
			break;
		case 'q':
			PostQueuedCompletionStatus(g_iocp, 0, 0, 0);
			return;
		}
	}
}

unsigned int __stdcall WorkerThread(LPVOID arg)
{
	while (1)
	{
		DWORD				transferredSize = 0;
		DWORD*				completionKey = 0;
		OVERLAPPED*			overlapped = 0;
		int					count = 0;

		PostQueuedCompletionStatus(g_iocp, 1, 0, 0);
		GetQueuedCompletionStatus(g_iocp, &transferredSize, (PULONG_PTR)&completionKey, &overlapped, INFINITE);
		
		if (transferredSize == 0)
		{
			PostQueuedCompletionStatus(g_iocp, 0, 0, 0);

			return 0;
		}

		CProfiler::Begin(L"GQCS");

		CProfiler::Begin(L"JOB_1");
		for (int i = 0; i < g_job1_count; ++i)
		{
			count++;
		}
		CProfiler::End(L"JOB_1");

		CProfiler::Begin(L"SLEEP");
		Sleep(g_sleep_count);
		CProfiler::End(L"SLEEP");

		CProfiler::Begin(L"JOB_2");
		for (int i = 0; i < g_job2_count; ++i)
		{
			count++;
		}
		CProfiler::End(L"JOB_2");

		CProfiler::End(L"GQCS");
	}
	return 0;
}
