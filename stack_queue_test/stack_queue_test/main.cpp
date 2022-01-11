#include <process.h>
#include <time.h>
#include "CProfiler.h"
#include <stack>
#include <queue>
#include "TC_LFQueue.h"
#include "TC_LFStack.h"
#include <stdio.h>
#include "CProfilerClock.h"

#define dfTHREAD_MAX (16)
#define dfCHUNK_SIZE (1000000)
#define dfTRY_NUM (1000)

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
bool g_spinStack;
TC_LFStack<int*> g_lfStack;

queue<int*> g_stdQueue;
SRWLOCK g_stdQueueLock;
bool g_spinQueue;
procademy::TC_LFQueue<int*> g_lfQueue;

int* g_data;
int g_thread_num;

void TestFunc(unsigned int WINAPI func(LPVOID arg));

int main()
{
	srand(time(NULL));
	//CProfiler::InitProfiler(100);
	procademy::CProfilerClock::InitProfilerClock();
	
	wprintf_s(L"Thread Num : ");
	scanf_s("%d", &g_thread_num);

	init();

	TestFunc(stdQueue);
	TestFunc(lfQueue);
	TestFunc(spinQueue);

	TestFunc(stdStack);
	TestFunc(lfStack);
	TestFunc(spinStack);

	//CProfiler::PrintAvg();
	procademy::CProfilerClock::PrintAvg();

	Destroy();
	procademy::CProfilerClock::DestroyProfiler();

	return 0;
}

unsigned int __stdcall stdQueue(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		procademy::CProfilerClock::Begin(L"stdQ");
		AcquireSRWLockExclusive(&g_stdQueueLock);
		localNums = g_stdQueue.front();
		g_stdQueue.pop();
		g_stdQueue.push(localNums);
		ReleaseSRWLockExclusive(&g_stdQueueLock);
		procademy::CProfilerClock::End(L"stdQ");
	}

	return 0;
}

unsigned int __stdcall lfQueue(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		procademy::CProfilerClock::Begin(L"lfQ");
		g_lfQueue.Dequeue(&localNums);
		g_lfQueue.Enqueue(localNums);
		procademy::CProfilerClock::End(L"lfQ");
	}

	return 0;
}

unsigned int __stdcall spinQueue(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfCHUNK_SIZE; ++t)
	{
		procademy::CProfilerClock::Begin(L"spinQ");
		while (InterlockedExchange8((char*)&g_spinQueue, true) == true)
		{
		}
		localNums = g_stdQueue.front();
		g_stdQueue.pop();
		g_stdQueue.push(localNums);
		g_spinQueue = false;
		procademy::CProfilerClock::End(L"spinQ");
	}

	return 0;
}

unsigned int __stdcall stdStack(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfTRY_NUM; ++t)
	{
		procademy::CProfilerClock::Begin(L"stdStack");
		AcquireSRWLockExclusive(&g_stdStackLock);
		localNums = g_stdStack.top();
		g_stdStack.pop();
		g_stdStack.push(localNums);
		ReleaseSRWLockExclusive(&g_stdStackLock);
		procademy::CProfilerClock::End(L"stdStack");
	}

	return 0;
}

unsigned int __stdcall lfStack(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfTRY_NUM; ++t)
	{
		procademy::CProfilerClock::Begin(L"lfStack");
		g_lfStack.Pop(&localNums);
		g_lfStack.Push(localNums);
		procademy::CProfilerClock::End(L"lfStack");
	}

	return 0;
}

unsigned int __stdcall spinStack(LPVOID arg)
{
	int* localNums;

	for (int t = 0; t < dfTRY_NUM; ++t)
	{
		procademy::CProfilerClock::Begin(L"spinStack");
		while (InterlockedExchange8((char*)&g_spinStack, true) == true)
		{
		}
		localNums = g_stdStack.top();
		g_stdStack.pop();
		g_stdStack.push(localNums);
		g_spinStack = false;
		procademy::CProfilerClock::End(L"spinStack");
	}

	return 0;
}

void init()
{
	InitializeSRWLock(&g_stdStackLock);
	InitializeSRWLock(&g_stdQueueLock);

	g_data = new int[g_thread_num * dfCHUNK_SIZE];

	for (int i = 0; i < g_thread_num * dfCHUNK_SIZE; ++i)
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
	HANDLE handles[dfTHREAD_MAX];

	for (int i = 0; i < g_thread_num; ++i)
	{
		handles[i] = (HANDLE)_beginthreadex(nullptr, 0, func, nullptr, 0, nullptr);
	}

	DWORD retval = WaitForMultipleObjects(g_thread_num, handles, true, INFINITE);

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
