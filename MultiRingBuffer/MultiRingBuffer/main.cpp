#include "RingBuffer.h"
#include <stdio.h>
#include <random>
#include <list>
#include <string>
#include <conio.h>
#include "QueuePacketDefine.h"
#include <iostream>

#define WORKER_SIZE (3)
#define WAIT_TIME (5)

using namespace std;

unsigned int __stdcall WorkerFunc(void* pvParam);

list<wstring> g_List;
RingBuffer g_msgQ(40000);
wstring g_msg = L"PROCADEMY";
HANDLE g_event;
SRWLOCK g_srwlockList;

int main()
{
	HANDLE hArray[WORKER_SIZE];
	srand((unsigned int)hArray);
	InitializeSRWLock(&g_srwlockList);

	for (int i = 0; i < WORKER_SIZE; ++i)
	{
		hArray[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerFunc, nullptr, 0, nullptr);
	}

	g_event = CreateEvent(nullptr, false, false, nullptr);

	bool shutdown = false;

	while (!shutdown)
	{
		st_MSG_HEAD header;
		header.shType = (rand() & 0xff) % 3;
		header.shStrLen = (rand() & 0x7) + 1;
		
		if (_kbhit())
		{
			char ch = _getch();

			if (ch == 32)
			{
				header.shType = dfTYPE_QUIT;
			}
		}

		g_msgQ.Enqueue((char*)&header, sizeof(header));

		DWORD retval = WaitForMultipleObjects(WORKER_SIZE, hArray, true, WAIT_TIME);

		switch (retval)
		{
		case WAIT_FAILED:
		{
			int err = GetLastError();
			wcout << L"Handle Error: " << err << endl;
			return -1;
		}
		case WAIT_OBJECT_0:
			wcout << L"END" << endl;
			shutdown = true;
			break;
		case WAIT_TIMEOUT:
			SetEvent(g_event);
			break;
		default:
			break;
		}

		Sleep(WAIT_TIME);
	}

	CloseHandle(g_event);
	for (int i = 0; i < WORKER_SIZE; ++i)
	{
		CloseHandle(hArray[i]);
	}

	return 0;
}

unsigned int __stdcall WorkerFunc(void* pvParam)
{
	while (1)
	{
		DWORD retval = WaitForSingleObject(g_event, INFINITE);

		switch (retval)
		{
		case WAIT_FAILED:
		case WAIT_TIMEOUT:
			wcout << L"Error\n" << endl;
			return -1;
		default:
			break;
		}

		g_msgQ.Lock(false);

		if (g_msgQ.GetUseSize() < sizeof(st_MSG_HEAD))
		{
			continue;
		}

		st_MSG_HEAD header;

		g_msgQ.Dequeue((char*)&header, sizeof(header));

		if (header.shType == dfTYPE_ADD_STR)
		{
			AcquireSRWLockExclusive(&g_srwlockList);
			g_List.push_back(g_msg.substr(0, header.shStrLen));
			ReleaseSRWLockExclusive(&g_srwlockList);
		}

		g_msgQ.Unlock(false);	

		if (g_msgQ.GetUseSize() > 0)
		{
			SetEvent(g_event);
		}

		switch (header.shType)
		{
		case dfTYPE_DEL_STR:
		{
			AcquireSRWLockExclusive(&g_srwlockList);
			if (g_List.size() > 0)
			{
				g_List.erase(--g_List.end());
			}
			ReleaseSRWLockExclusive(&g_srwlockList);
				
			break;
		}
		case dfTYPE_PRINT_LIST:
		{
			AcquireSRWLockShared(&g_srwlockList);
			wprintf_s(L"List: ");
			for (auto iter = g_List.begin(); iter != g_List.end(); ++iter)
			{
				wprintf_s(L"[%s] ", iter->c_str());
			}
			wprintf_s(L"\n");
			ReleaseSRWLockShared(&g_srwlockList);
			
			break;
		}
		case dfTYPE_QUIT:
			return 0;
		default:
			break;
		}
	}

	return 0;
}
