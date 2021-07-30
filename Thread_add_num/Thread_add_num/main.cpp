#include <process.h>
#include <Windows.h>
#include <stdio.h>
#include <time.h>

int g_Data = 0;
int g_Connect = 0;
bool g_Shutdown = false;
HANDLE g_TimerEvent;

unsigned int __stdcall AcceptThreadFunc(void* pvParam);
unsigned int __stdcall DisconnectThreadFunc(void* pvParam);
unsigned int __stdcall UpdateThreadFunc(void* pvParam);

int main()
{
	unsigned int acceptThreadID;
	unsigned int disconnectThreadID;
	unsigned int updateThreadID1;
	unsigned int updateThreadID2;
	unsigned int updateThreadID3;

	g_TimerEvent = CreateEvent(nullptr, true, false, nullptr);
	HANDLE acceptThread = (HANDLE)_beginthreadex(nullptr, 0, AcceptThreadFunc, nullptr, 0, &acceptThreadID);
	HANDLE disconnectThread = (HANDLE)_beginthreadex(nullptr, 0, DisconnectThreadFunc, nullptr, 0, &disconnectThreadID);
	HANDLE updateThread1 = (HANDLE)_beginthreadex(nullptr, 0, UpdateThreadFunc, nullptr, 0, &updateThreadID1);
	HANDLE updateThread2 = (HANDLE)_beginthreadex(nullptr, 0, UpdateThreadFunc, nullptr, 0, &updateThreadID2);
	HANDLE updateThread3 = (HANDLE)_beginthreadex(nullptr, 0, UpdateThreadFunc, nullptr, 0, &updateThreadID3);

	HANDLE hArray[5] = { acceptThread, disconnectThread, updateThread1, updateThread2, updateThread3 };

	int count = 0;

	while (count < 20)
	{
		WaitForSingleObject(g_TimerEvent, 1000);
		count++;
		wprintf_s(L"g_Connect: %d\n", g_Connect);
	}

	g_Shutdown = true;
	
	DWORD retval = WaitForMultipleObjects(5, hArray, true, INFINITE);

	CloseHandle(g_TimerEvent);
	for (int i = 0; i < 5; ++i)
	{
		CloseHandle(hArray[i]);
	}

	switch (retval)
	{
	case WAIT_FAILED:
		wprintf_s(L"Handle Error: %d\n", GetLastError());
		return -1;
	case WAIT_TIMEOUT:
		wprintf_s(L"Timeout : %d\n", GetLastError());
		return -1;
	default:
		return 0;
	}
}

unsigned int __stdcall AcceptThreadFunc(void* pvParam)
{
	srand(time(nullptr) + 1000);
	while (g_Shutdown == false)
	{
		int ranNum = rand() % 900 + 100;

		WaitForSingleObject(g_TimerEvent, ranNum);

		int retval = InterlockedIncrement((LONG*)&g_Connect);
	}

	return 0;
}

unsigned int __stdcall DisconnectThreadFunc(void* pvParam)
{
	srand(time(nullptr) + 3000);
	while (g_Shutdown == false)
	{
		int ranNum = rand() % 900 + 100;

		WaitForSingleObject(g_TimerEvent, ranNum);

		int retval = InterlockedDecrement((LONG*)&g_Connect);
	}

	return 0;
}

unsigned int __stdcall UpdateThreadFunc(void* pvParam)
{
	while (g_Shutdown == false)
	{
		WaitForSingleObject(g_TimerEvent, 10);

		int retval = InterlockedIncrement((LONG*)&g_Data);

		if (retval % 1000 == 0)
		{
			wprintf_s(L"g_Data: %d\n", retval);
		}
	}

	return 0;
}
