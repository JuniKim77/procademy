#include "CSafeQueue.h"
#include <stdio.h>
#include <cstring>
#include <random>
#include <conio.h>
#include <stdlib.h>
#include <process.h>

#define STR_SIZE (120)
#define TIME_PERIOD (50)
#define BUFFER_SIZE (150)

unsigned int __stdcall dequeueProc(void* pvParam);
unsigned int __stdcall enqueueProc(void* pvParam);
int dequeueProcess();
int enqueueProcess();

char szTest[] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123456";
int cur = 0;
int post = 0;

procademy::CSafeQueue<char> g_q(BUFFER_SIZE);
HANDLE g_event;

int main()
{
	system(" mode  con lines=30   cols=120 ");
	int seed = 10;

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

	return 0;
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

int dequeueProcess()
{
	char buffer[STR_SIZE + 1];

	int ran = rand() % (STR_SIZE + 1);

	int size = 0;
	char ch = 0;

	if (ran == 0)
		return 0;

	while ((ch = g_q.Dequeue()) != NULL)
	{
		buffer[size++] = ch;

		if (size == ran)
		{
			break;
		}
	}

	buffer[size] = '\0';

	if (size > 0)
		printf(buffer);

	return size;
}

int enqueueProcess()
{
	char buffer[STR_SIZE + 1];

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
	int size = 0;

	for (size = 0; size < ran; ++size)
	{
		if (g_q.Enqueue(buffer[size]) == false)
		{
			break;
		}
	}

	cur = (cur + size) % STR_SIZE;

	return size;
}
