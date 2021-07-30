#include <conio.h>
#include <process.h>
#include <Windows.h>
#include <stdio.h>
#include <list>
#include <time.h>

std::list<int> g_nums;
SRWLOCK g_srwNums;
HANDLE g_SaveEvent;
HANDLE g_ExitEvent;

unsigned int __stdcall PrintThreadFunc(void* pvParam);
unsigned int __stdcall DeleteThreadFunc(void* pvParam);
unsigned int __stdcall WorkerThreadFunc(void* pvParam);
unsigned int __stdcall SaveThreadFunc(void* pvParam);

int main()
{
	InitializeSRWLock(&g_srwNums);
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
		char ch = _getch();
		rewind(stdin);

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
	while (1)
	{
		DWORD retval = WaitForSingleObject(g_ExitEvent, 1000);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			AcquireSRWLockShared(&g_srwNums);

			if (g_nums.size() > 0)
			{
				auto iter = g_nums.begin();

				wprintf_s(L"%d", *iter);
				iter++;

				for (; iter != g_nums.end(); ++iter)
				{
					wprintf_s(L"-%d", *iter);
				}
				wprintf_s(L"\n");
			}

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
	while (1)
	{
		DWORD retval = WaitForSingleObject(g_ExitEvent, 333);

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

	while (1)
	{
		DWORD retval = WaitForSingleObject(g_ExitEvent, 1000);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			int num = rand();

			AcquireSRWLockExclusive(&g_srwNums);

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
	HANDLE hArray[2] = { g_ExitEvent, g_SaveEvent };

	while (1)
	{
		DWORD retval = WaitForMultipleObjects(2, hArray, false, INFINITE);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			wprintf_s(L"Timeout Error %d\n", GetLastError());
			break;
		}
		case WAIT_OBJECT_0:
			return 0;
		case WAIT_OBJECT_0 + 1:
		{
			AcquireSRWLockShared(&g_srwNums);

			if (g_nums.size() > 0)
			{
				FILE* fout;
				_wfopen_s(&fout, L"result.txt", L"a");

				auto iter = g_nums.begin();

				fwprintf_s(fout, L"%d", *iter);
				iter++;

				for (; iter != g_nums.end(); ++iter)
				{
					fwprintf_s(fout, L"-%d", *iter);
				}

				fwprintf_s(fout, L"\n");
				fclose(fout);
			}

			ReleaseSRWLockShared(&g_srwNums);

			break;
		}
		default:
			break;
		}
	}

	return 0;
}
