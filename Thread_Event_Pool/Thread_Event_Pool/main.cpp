#include <process.h>
#include <Windows.h>
#include <stdio.h>
#include <conio.h>

HANDLE hThreadEvent;
HANDLE hExitEvent;

unsigned int __stdcall PrintMyself(void* pvParam);

int main()
{
	HANDLE hArray[20];
	hThreadEvent = CreateEvent(nullptr, false, false, nullptr);
	hExitEvent = CreateEvent(nullptr, true, false, nullptr);

	for (int i = 0; i < 20; ++i)
	{
		hArray[i] = (HANDLE)_beginthreadex(nullptr, 0, PrintMyself, nullptr, 0, nullptr);
	}

	while (1)
	{
		char ch = _getch();

		if (ch == 'x')
		{
			SetEvent(hThreadEvent);
		}
		if (ch == 'q')
		{
			SetEvent(hExitEvent);
			break;
		}
	}

	WaitForMultipleObjects(20, hArray, true, INFINITE);

	CloseHandle(hThreadEvent);
	CloseHandle(hExitEvent);
	for (int i = 0; i < 20; ++i)
	{
		CloseHandle(hArray[i]);
	}

	return 0;
}

unsigned int __stdcall PrintMyself(void* pvParam)
{
	HANDLE hArray[2] = { hExitEvent, hThreadEvent };

	while (1)
	{
		DWORD retval = WaitForMultipleObjects(2, hArray, false, INFINITE);

		switch (retval)
		{
		case WAIT_FAILED:
		case WAIT_TIMEOUT:
			wprintf_s(L"Error %d\n", GetLastError());
			return -1;
		case WAIT_OBJECT_0:
			return 0;
		case WAIT_OBJECT_0 + 1:
			wprintf_s(L"Current Thread ID: %d\n", GetCurrentThreadId());
			break;
		default:
			break;
		}
	}

	return 0;
}
