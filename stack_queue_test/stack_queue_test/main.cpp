#include <process.h>
#include <time.h>
//#include "CProfiler.h"
#include <stack>
#include <queue>
#include "TC_LFQueue.h"
//#include "TC_LFQueue64.h"
#include "TC_LFStack.h"
#include <stdio.h>
#include "CProfilerClock.h"
#include <timeapi.h>
#include "CSafeQueue.h"

#pragma comment(lib, "winmm")

#define dfTHREAD_MAX (32)
//#define dfDEBUG

using namespace std;

unsigned int WINAPI stdQueue(LPVOID arg);
unsigned int WINAPI lfQueue(LPVOID arg);
unsigned int WINAPI spinQueue(LPVOID arg);
unsigned int WINAPI stdStack(LPVOID arg);
unsigned int WINAPI lfStack(LPVOID arg);
unsigned int WINAPI spinStack(LPVOID arg);
void init();

stack<int*> g_stdStack;
SRWLOCK g_stdStackLock;
bool g_spinStack;
TC_LFStack<int*> g_lfStack;

queue<int*> g_stdQueue;
procademy::CSafeQueue<int*> g_safeQ;
SRWLOCK g_stdQueueLock;
bool g_spinQueue;
procademy::TC_LFQueue<int*> g_lfQueue;

int g_thread_num;
volatile int g_test_size = 1000;
int* g_data;

enum DEBUG_TYPE
{
	STL_Q,
	SPIN_Q,
	LF_Q,
	STL_STACK,
	SPIN_STACK,
	LF_STACK
};

struct testDebug
{
	DWORD threadID;
	DEBUG_TYPE type;
};

testDebug g_debug[USHRT_MAX];
USHORT g_idx = USHRT_MAX;

HANDLE g_event;

void _log(
	DEBUG_TYPE type
)
{
	USHORT index = InterlockedIncrement16((short*)&g_idx);

	g_debug[index].threadID = GetCurrentThreadId();
	g_debug[index].type = type;
}

void TestFunc(unsigned int WINAPI func(LPVOID arg));

int main()
{
	timeBeginPeriod(1);

	g_event = CreateEvent(nullptr, true, false, nullptr);

	srand(time(NULL));
	//CProfiler::InitProfiler(100);
	
	wprintf_s(L"Thread Num : ");
	scanf_s("%d", &g_thread_num);

	wprintf_s(L"Test Size : ");
	scanf_s("%d", &g_test_size);

	int op = 0;

	init();

	TestFunc(stdQueue);
	TestFunc(lfQueue);
	TestFunc(spinQueue);

	TestFunc(stdStack);
	TestFunc(lfStack);
	TestFunc(spinStack);

	//CProfiler::PrintAvg();
	procademy::CProfilerClock::PrintAvg();

	timeEndPeriod(1);

	return 0;
}

unsigned int __stdcall stdQueue(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	volatile int count = 0;

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"stdQ");
		AcquireSRWLockExclusive(&g_stdQueueLock);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::STL_Q);
#endif		
		//localNums = g_stdQueue.front();
		//g_stdQueue.pop();
		localNums = g_safeQ.Dequeue();
		ReleaseSRWLockExclusive(&g_stdQueueLock);
		procademy::CProfilerClock::End(L"stdQ");
		
		for (int i = 0; i < 100; ++i)
		{
			count++;
		}

		procademy::CProfilerClock::Begin(L"stdQ");
		AcquireSRWLockExclusive(&g_stdQueueLock);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::STL_Q);
#endif		
		//g_stdQueue.push(localNums);
		g_safeQ.Enqueue(localNums);
		ReleaseSRWLockExclusive(&g_stdQueueLock);
		procademy::CProfilerClock::End(L"stdQ");

		for (int i = 0; i < 100; ++i)
		{
			count++;
		}
	}

	return 0;
}

unsigned int __stdcall lfQueue(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"lfQ");
		g_lfQueue.Dequeue(&localNums);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::LF_Q);
#endif	
		procademy::CProfilerClock::End(L"lfQ");

		procademy::CProfilerClock::Begin(L"lfQ");
		g_lfQueue.Enqueue(localNums);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::LF_Q);
#endif	
		procademy::CProfilerClock::End(L"lfQ");
	}

	return 0;
}

unsigned int __stdcall spinQueue(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"spinQ");
		while (InterlockedExchange8((char*)&g_spinQueue, true) == true)
		{
		}
#ifdef dfDEBUG
		_log(DEBUG_TYPE::SPIN_Q);
#endif	
		/*localNums = g_stdQueue.front();
		g_stdQueue.pop();*/
		localNums = g_safeQ.Dequeue();
		g_spinQueue = false;
		procademy::CProfilerClock::End(L"spinQ");

		procademy::CProfilerClock::Begin(L"spinQ");
		while (InterlockedExchange8((char*)&g_spinQueue, true) == true)
		{
		}
#ifdef dfDEBUG
		_log(DEBUG_TYPE::SPIN_Q);
#endif	
		//g_stdQueue.push(localNums);
		g_safeQ.Enqueue(localNums);
		g_spinQueue = false;
		procademy::CProfilerClock::End(L"spinQ");
	}

	return 0;
}

unsigned int __stdcall stdStack(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	volatile int count = 0;

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"stdStack");
		AcquireSRWLockExclusive(&g_stdStackLock);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::STL_STACK);
#endif	
		localNums = g_stdStack.top();
		g_stdStack.pop();
		ReleaseSRWLockExclusive(&g_stdStackLock);
		procademy::CProfilerClock::End(L"stdStack");

		for (int i = 0; i < 100; ++i)
		{
			count++;
		}

		procademy::CProfilerClock::Begin(L"stdStack");
		AcquireSRWLockExclusive(&g_stdStackLock);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::STL_STACK);
#endif	
		g_stdStack.push(localNums);
		ReleaseSRWLockExclusive(&g_stdStackLock);
		procademy::CProfilerClock::End(L"stdStack");

		for (int i = 0; i < 100; ++i)
		{
			count++;
		}
	}

	return 0;
}

unsigned int __stdcall lfStack(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"lfStack");
		g_lfStack.Pop(&localNums);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::LF_STACK);
#endif	
		procademy::CProfilerClock::End(L"lfStack");

		procademy::CProfilerClock::Begin(L"lfStack");
		g_lfStack.Push(localNums);
#ifdef dfDEBUG
		_log(DEBUG_TYPE::LF_STACK);
#endif	
		procademy::CProfilerClock::End(L"lfStack");
	}

	return 0;
}

unsigned int __stdcall spinStack(LPVOID arg)
{
	int* localNums;

	WaitForSingleObject(g_event, INFINITE);

	for (int t = 0; t < g_test_size; ++t)
	{
		procademy::CProfilerClock::Begin(L"spinStack");
		while (InterlockedExchange8((char*)&g_spinStack, true) == true)
		{
		}
#ifdef dfDEBUG
		_log(DEBUG_TYPE::SPIN_STACK);
#endif	
		localNums = g_stdStack.top();
		g_stdStack.pop();
		g_spinStack = false;
		procademy::CProfilerClock::End(L"spinStack");

		procademy::CProfilerClock::Begin(L"spinStack");
		while (InterlockedExchange8((char*)&g_spinStack, true) == true)
		{
		}
#ifdef dfDEBUG
		_log(DEBUG_TYPE::SPIN_STACK);
#endif	
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

	g_data = new int[100];

	for (int i = 0; i < 100; ++i)
	{
		g_stdQueue.push(&g_data[i]);
		g_lfQueue.Enqueue(&g_data[i]);
		g_stdStack.push(&g_data[i]);
		g_lfStack.Push(&g_data[i]);
		g_safeQ.Enqueue(&g_data[i]);
	}
}

void TestFunc(unsigned int __stdcall func(LPVOID arg))
{
	HANDLE handles[dfTHREAD_MAX];

	for (int i = 0; i < g_thread_num; ++i)
	{
		handles[i] = (HANDLE)_beginthreadex(nullptr, 0, func, nullptr, 0, nullptr);
	}

	Sleep(1);

	SetEvent(g_event);

	DWORD retval = WaitForMultipleObjects(g_thread_num, handles, true, INFINITE);

	ResetEvent(g_event);

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
