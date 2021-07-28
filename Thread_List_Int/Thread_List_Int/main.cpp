#include <process.h>
#include <Windows.h>
#include <stdio.h>
#include <list>
#include <time.h>

std::list<int> g_nums;
SRWLOCK g_srwNums;
HANDLE g_TimerEvent;
HANDLE g_SaveEvent;
HANDLE g_ExitEvent;

unsigned int __stdcall PrintThreadFunc(void* pvParam);
unsigned int __stdcall DeleteThreadFunc(void* pvParam);
unsigned int __stdcall WorkerThreadFunc(void* pvParam);
unsigned int __stdcall SaveThreadFunc(void* pvParam);

int main()
{
	InitializeSRWLock(&g_srwNums);
	g_TimerEvent = CreateEvent(nullptr, true, false, nullptr);
	g_SaveEvent = CreateEvent(nullptr, false, false, nullptr);
	g_ExitEvent = CreateEvent(nullptr, true, false, nullptr);
	HANDLE hArray[6];

	hArray[0] = (HANDLE)_beginthreadex(nullptr, 0, PrintThreadFunc, nullptr, 0, nullptr);
	hArray[1] = (HANDLE)_beginthreadex(nullptr, 0, DeleteThreadFunc, nullptr, 0, nullptr);
	hArray[2] = (HANDLE)_beginthreadex(nullptr, 0, SaveThreadFunc, nullptr, 0, nullptr);
	for (int i = 3; i < 6; ++i)
	{
		hArray[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThreadFunc, nullptr, 0, nullptr);
	}

	while (1)
	{
		char ch = getchar();

		if (ch == 'x')
		{
			SetEvent(g_SaveEvent);
		}
		if (ch == 'q')
		{
			SetEvent(g_ExitEvent);
			break;
		}
	}

	WaitForMultipleObjects(6, hArray, true, INFINITE);

	CloseHandle(g_TimerEvent);
	CloseHandle(g_SaveEvent);
	CloseHandle(g_ExitEvent);
	for (int i = 0; i < 6; ++i)
	{
		CloseHandle(hArray[i]);
	}

	return 0;
}

unsigned int __stdcall PrintThreadFunc(void* pvParam)
{
	HANDLE hArray[2] = { g_ExitEvent, g_TimerEvent };

	while (1)
	{
		DWORD retval = WaitForMultipleObjects(2, hArray, false, 1000);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			AcquireSRWLockShared(&g_srwNums);

			if (g_nums.size() == 0)
			{
				break;
			}

			auto iter = g_nums.begin();

			wprintf_s(L"%d", *iter);
			iter++;

			for (; iter != g_nums.end(); ++iter)
			{
				wprintf_s(L"-%d", *iter);
			}
			wprintf_s(L"\n");

			ReleaseSRWLockShared(&g_srwNums);
			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		default:
			break;
		}
	}

	return 0;
}

unsigned int __stdcall DeleteThreadFunc(void* pvParam)
{
	HANDLE hArray[2] = { g_ExitEvent, g_TimerEvent };

	while (1)
	{
		DWORD retval = WaitForMultipleObjects(2, hArray, false, 333);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			AcquireSRWLockExclusive(&g_srwNums);

			if (g_nums.size() > 0)
			{
				auto iter = g_nums.end();
				iter--;

				g_nums.erase(iter);
			}

			ReleaseSRWLockExclusive(&g_srwNums);
			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		default:
			break;
		}
	}

	return 0;
}

unsigned int __stdcall WorkerThreadFunc(void* pvParam)
{
	srand(time(nullptr));

	HANDLE hArray[2] = { g_ExitEvent, g_TimerEvent };

	while (1)
	{
		DWORD retval = WaitForMultipleObjects(2, hArray, false, 1000);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			AcquireSRWLockExclusive(&g_srwNums);

			int num = rand();

			g_nums.push_back(num);

			ReleaseSRWLockExclusive(&g_srwNums);
			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		default:
			break;
		}
	}

	return 0;
}

unsigned int __stdcall SaveThreadFunc(void* pvParam)
{
	FILE* fout;
	_wfopen_s(&fout, L"result.txt", L"a");

	AcquireSRWLockShared(&g_srwNums);

	if (g_nums.size() == 0)
	{
		fclose(fout);
		return 0;
	}

	auto iter = g_nums.begin();

	fwprintf_s(fout, L"%d", *iter);
	iter++;

	for (; iter != g_nums.end(); ++iter)
	{
		fwprintf_s(fout, L"-%d", *iter);
	}

	fwprintf_s(fout, L"\n");

	ReleaseSRWLockShared(&g_srwNums);

	fclose(fout);

	return 0;
}
