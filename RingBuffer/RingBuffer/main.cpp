#include "RingBuffer.h"
#include <stdio.h>
#include <cstring>
#include <random>
#include <conio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

#define STR_SIZE (120)
#define TIME_PERIOD (50)
unsigned int __stdcall dequeueProc(void* pvParam);
unsigned int __stdcall enqueueProc(void* pvParam);
int dequeueProcess();
int enqueueProcess();

RingBuffer ringBuffer(BUFFER_SIZE);
HANDLE g_event;

char szTest[] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123456";
int cur = 0;
int post = 0;

int main()
{
	CProfiler::InitProfiler(5);
	procademy::CCrashDump::CCrashDump();

	ringBuffer.SetLogMode(false);

	system(" mode  con lines=30   cols=120 ");
	int seed = 10;
	int ringSize = BUFFER_SIZE;

	srand(seed);

	HANDLE hArray[2];
	hArray[0] = (HANDLE)_beginthreadex(nullptr, 0, enqueueProc, nullptr, 0, nullptr);
	hArray[1] = (HANDLE)_beginthreadex(nullptr, 0, dequeueProc, nullptr, 0, nullptr);

	g_event = CreateEvent(nullptr, false, false, nullptr);

	while (1)
	{
		char ch = _getch();
		rewind(stdin);

		if (ch == 'q')
		{
			SetEvent(g_event);
			break;
		}
	}

	WaitForMultipleObjects(2, hArray, true, INFINITE);

	CloseHandle(g_event);
	for (int i = 0; i < 2; ++i)
	{
		CloseHandle(hArray[i]);
	}

	CProfiler::Print();

	return 0;
}

int dequeueProcess()
{
	char buffer[STR_SIZE + 1];
	WCHAR wbuffer[STR_SIZE + 1];

	int ran = rand() % (STR_SIZE + 1);

	CProfiler::Begin(L"GetUseSize");
	if (ringBuffer.GetUseSize() == 0)
	{
		CProfiler::End(L"GetUseSize");
		return 0;
	}
	CProfiler::End(L"GetUseSize");

	CProfiler::Begin(L"Dequeue");
	int size = ringBuffer.Dequeue(buffer, ran);
	CProfiler::End(L"Dequeue");

	buffer[size] = '\0';

	printf(buffer);

	//if (size != 0 && szTest[post] != buffer[0])
	//{
	//	//CDebugger::PrintLogOut(L"DEBUG");
	//	CRASH();
	//}

	//post = (post + size) % STR_SIZE;
	//CDebugger::_Log(L"[Post: %d]", post);

	return size;
}

int enqueueProcess()
{
	char buffer[STR_SIZE + 1];
	WCHAR wbuffer[STR_SIZE + 1];

	int ran = rand() % (STR_SIZE + 1);

	if (cur + ran > STR_SIZE)
	{
		int poss = STR_SIZE - cur;
		memcpy(buffer, szTest + cur, poss);
		memcpy(buffer + poss, szTest, ran - poss);
	}
	else
	{
		memcpy(buffer, szTest + cur, ran);
	}

	buffer[ran] = '\0';

	CProfiler::Begin(L"Enqueue");
	int size = ringBuffer.Enqueue(buffer, ran);
	CProfiler::End(L"Enqueue");

	cur = (cur + size) % STR_SIZE;

	return size;
}

unsigned int __stdcall dequeueProc(void* pvParam)
{
	srand(100);

	while (1)
	{
		DWORD time = rand() % TIME_PERIOD + 10;

		DWORD retval = WaitForSingleObject(g_event, time);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error: %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			dequeueProcess();
			break;
		}
		case WAIT_OBJECT_0:
			SetEvent(g_event);
			return 0;
		default:
			break;
		}
	}
	return 0;
}

unsigned int __stdcall enqueueProc(void* pvParam)
{
	srand(1000);

	while (1)
	{
		DWORD time = rand() % TIME_PERIOD + 10;

		DWORD retval = WaitForSingleObject(g_event, time);

		switch (retval)
		{
		case WAIT_FAILED:
			wprintf_s(L"Handle Error: %d\n", GetLastError());
			return -1;
		case WAIT_TIMEOUT:
		{
			enqueueProcess();
			break;
		}
		case WAIT_OBJECT_0:
			SetEvent(g_event);
			return 0;
		default:
			break;
		}
	}

	return 0;
}
