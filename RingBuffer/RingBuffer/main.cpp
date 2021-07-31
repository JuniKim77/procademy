#include "RingBuffer.h"
#include <stdio.h>
#include <cstring>
#include <random>
#include <conio.h>

#define STR_SIZE (120)
#define TIME_PERIOD (50)
unsigned int __stdcall dequeueProc(void* pvParam);
unsigned int __stdcall enqueueProc(void* pvParam);
int dequeueProcess();
int enqueueProcess();

RingBuffer ringBuffer(BUFFER_SIZE);
HANDLE g_event;
SRWLOCK g_srwlock;

char szTest[] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123456";
int cur = 0;

int main()
{
	system(" mode  con lines=30   cols=120 ");

	/*printf("Seed: ");
	int seed = 10;
	scanf_s("%d", &seed);
	printf("Ring Buffer Size: ");
	int ringSize = BUFFER_SIZE;
	scanf_s("%d", &ringSize);*/
	int seed = 10;

	srand(seed);

	HANDLE hArray[2];
	hArray[0] = (HANDLE)_beginthreadex(nullptr, 0, enqueueProc, nullptr, 0, nullptr);
	hArray[1] = (HANDLE)_beginthreadex(nullptr, 0, dequeueProc, nullptr, 0, nullptr);
	InitializeSRWLock(&g_srwlock);

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

int dequeueProcess()
{
	char buffer[STR_SIZE + 1];

	int ran = rand() % (STR_SIZE + 1);

	/*int dDequeSize = ringBuffer->DirectDequeueSize();

	if (ran <= dDequeSize)
	{
		memcpy(buffer, ringBuffer->GetFrontBufferPtr(), dDequeSize);
		buffer[dDequeSize] = '\0';
		ringBuffer->MoveFront(dDequeSize);

		return dDequeSize;
	}
	else
	{

	}*/
	//ringBuffer.Lock(false);
	if (ringBuffer.GetUseSize() == 0)
	{
		ringBuffer.Unlock(false);
		return 0;
	}

	int size = ringBuffer.Dequeue(buffer, ran);
	//ringBuffer.Unlock(false);

	buffer[size] = '\0';
	printf(buffer);

	return size;
}

int enqueueProcess()
{
	char buffer[STR_SIZE + 1];

	// 랜덤 숫자 맞큼 넣을 것
	int ran = rand() % (STR_SIZE + 1);

	// 램덤 숫자와 현재 위치의 합이 문자열 크기보다 크면...
	//if (cur + ran > STR_SIZE)
	//{
	//	// 가능한 만큼 미리 복사하고... 나머지 따로 복사
	//	int poss = STR_SIZE - cur;
	//	memcpy(buffer, szTest + cur, poss);
	//	memcpy(buffer + poss, szTest, ran - poss);
	//}
	//else
	//{
	//	memcpy(buffer, szTest + cur, ran);
	//}

	//buffer[ran] = '\0';

	//int dEneueSize = ringBuffer.DirectEnqueueSize();

	//if (ran < dEneueSize)
	//{
	//	memcpy(ringBuffer.GetRearBufferPtr(), buffer, ran);

	//	ringBuffer.MoveRear(ran);

	//	cur = (cur + ran) % STR_SIZE;

	//	return ran;
	//}
	//else
	//{
	//	memcpy(ringBuffer.GetRearBufferPtr(), buffer, dEneueSize);

	//	ringBuffer.MoveRear(dEneueSize);

	//	if (ringBuffer.IsFrontZero())
	//	{
	//		cur = (cur + dEneueSize) % (STR_SIZE + 1);
	//	}
	//	else
	//	{
	//		cur = (cur + dEneueSize) % STR_SIZE;
	//	}

	//	return dEneueSize;
	//}

	if (cur + ran > STR_SIZE)
	{
		// 가능한 만큼 미리 복사하고... 나머지 따로 복사
		int poss = STR_SIZE - cur;
		memcpy(buffer, szTest + cur, poss);
		memcpy(buffer + poss, szTest, ran - poss);
	}
	else
	{
		memcpy(buffer, szTest + cur, ran);
	}

	buffer[ran] = '\0';

	//ringBuffer.Lock(false);
	int size = ringBuffer.Enqueue(buffer, ran);
	//ringBuffer.Unlock(false);

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
