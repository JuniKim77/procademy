#include <process.h>
#include <time.h>
#include "CProfiler.h"
#include <stack>
#include <queue>
#include "TC_LFQueue.h"
#include "TC_LFStack.h"
#include <stdio.h>

#define dfTHREAD_NUM (8)
#define dfCHUNK_SIZE (1000)

using namespace std;

unsigned int WINAPI stdQueue(LPVOID arg);
unsigned int WINAPI lfQueue(LPVOID arg);
unsigned int WINAPI spinQueue(LPVOID arg);
unsigned int WINAPI stdStack(LPVOID arg);
unsigned int WINAPI lfStack(LPVOID arg);
unsigned int WINAPI spinStack(LPVOID arg);
void init();
void Destroy();

stack<int*> g_stdStack;
SRWLOCK g_stdStackLock;
alignas(64) bool g_spinStack;
TC_LFStack<int*> g_lfStack;

queue<int*> g_stdQueue;
SRWLOCK g_stdQueueLock;
alignas(64) bool g_spinQueue;
procademy::TC_LFQueue<int*> g_lfQueue;

int* g_data;

void TestFunc(unsigned int WINAPI func(LPVOID arg));

int main()
{
	srand(time(NULL));
	CProfiler::InitProfiler(100);
	HANDLE handles[dfTHREAD_NUM];

	init();

	TestFunc(stdQueue);
	TestFunc(lfQueue);
	TestFunc(spinQueue);

	TestFunc(stdStack);
	TestFunc(lfStack);
	TestFunc(spinStack);

	CProfiler::PrintAvg();

	Destroy();

	return 0;
}

unsigned int __stdcall stdQueue(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"stdQ");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			AcquireSRWLockExclusive(&g_stdQueueLock);
			localNums[i] = g_stdQueue.front();
			g_stdQueue.pop();
			ReleaseSRWLockExclusive(&g_stdQueueLock);
		}

		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			AcquireSRWLockExclusive(&g_stdQueueLock);
			g_stdQueue.push(localNums[i]);
			ReleaseSRWLockExclusive(&g_stdQueueLock);
		}
		CProfiler::End(L"stdQ");
	}

	return 0;
}

unsigned int __stdcall lfQueue(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"lfQ");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			g_lfQueue.Dequeue(&localNums[i]);
		}
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			g_lfQueue.Enqueue(localNums[i]);
		}
		CProfiler::End(L"lfQ");
	}

	return 0;
}

unsigned int __stdcall spinQueue(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"spinQ");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			while (InterlockedExchange8((char*)&g_spinQueue, true) == true)
			{
			}
			localNums[i] = g_stdQueue.front();
			g_stdQueue.pop();
			g_spinQueue = false;
		}
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			while (InterlockedExchange8((char*)&g_spinQueue, true) == true)
			{
			}
			g_stdQueue.push(localNums[i]);
			g_spinQueue = false;
		}
		CProfiler::End(L"spinQ");
	}

	return 0;
}

unsigned int __stdcall stdStack(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"stdStack");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			AcquireSRWLockExclusive(&g_stdStackLock);
			localNums[i] = g_stdStack.top();
			g_stdStack.pop();
			ReleaseSRWLockExclusive(&g_stdStackLock);
		}
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			AcquireSRWLockExclusive(&g_stdStackLock);
			g_stdStack.push(localNums[i]);
			ReleaseSRWLockExclusive(&g_stdStackLock);
		}
		CProfiler::End(L"stdStack");
	}

	return 0;
}

unsigned int __stdcall lfStack(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"lfStack");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			g_lfStack.Pop(&localNums[i]);
		}
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			g_lfStack.Push(localNums[i]);
		}
		CProfiler::End(L"lfStack");
	}

	return 0;
}

unsigned int __stdcall spinStack(LPVOID arg)
{
	int* localNums[dfCHUNK_SIZE];

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		CProfiler::Begin(L"spinStack");
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			while (InterlockedExchange8((char*)&g_spinStack, true) == true)
			{
			}
			localNums[i] = g_stdStack.top();
			g_stdStack.pop();
			g_spinStack = false;
		}
		for (int i = 0; i < dfCHUNK_SIZE; ++i)
		{
			while (InterlockedExchange8((char*)&g_spinStack, true) == true)
			{
			}
			g_stdStack.push(localNums[i]);
			g_spinStack = false;
		}
		CProfiler::End(L"spinStack");
	}

	return 0;
}

void init()
{
	InitializeSRWLock(&g_stdStackLock);
	InitializeSRWLock(&g_stdQueueLock);

	g_data = new int[dfTHREAD_NUM * dfCHUNK_SIZE];

	for (int i = 0; i < dfTHREAD_NUM * dfCHUNK_SIZE; ++i)
	{
		g_stdQueue.push(&g_data[i]);
		g_lfQueue.Enqueue(&g_data[i]);
		g_stdStack.push(&g_data[i]);
		g_lfStack.Push(&g_data[i]);
	}
}

void Destroy()
{
	delete[] g_data;
}

void TestFunc(unsigned int __stdcall func(LPVOID arg))
{
	HANDLE handles[dfTHREAD_NUM];

	for (int i = 0; i < dfTHREAD_NUM; ++i)
	{
		handles[i] = (HANDLE)_beginthreadex(nullptr, 0, func, nullptr, 0, nullptr);
	}

	DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, handles, true, INFINITE);

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
}
