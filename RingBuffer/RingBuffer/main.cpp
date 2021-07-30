#include "RingBuffer.h"
#include <stdio.h>
#include <cstring>
#include <random>

#define STR_SIZE (120)

int dequeueProc(RingBuffer* ringBuffer);
int enqueueProc(RingBuffer* ringBuffer);

char szTest[] = "1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 1234567890 abcdefghijklmnopqrstuvwxyz 123456";
int cur = 0;

int main()
{
	system(" mode  con lines=30   cols=120 ");
	int seed = 10;
	int ringSize = BUFFER_SIZE;
	//printf("Seed: ");
	//int seed = 10;
	//scanf_s("%d", &seed);
	//printf("Ring Buffer Size: ");
	//int ringSize = BUFFER_SIZE;
	//scanf_s("%d", &ringSize);

	RingBuffer ringBuffer(ringSize);

	srand(seed);

	while (1)
	{
		int ran = enqueueProc(&ringBuffer);

		printf("After Enqueue::: Ran: %d\n", ran);
		ringBuffer.printInfo();

		ran = dequeueProc(&ringBuffer);

		printf("After Enqueue::: Ran: %d\n", ran);
		ringBuffer.printInfo();
	}

	return 0;
}

int dequeueProc(RingBuffer* ringBuffer)
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

	int size = ringBuffer->Dequeue(buffer, ran);

	buffer[size] = '\0';
	//printf(buffer);

	return size;
}

int enqueueProc(RingBuffer* ringBuffer)
{
	char buffer[STR_SIZE + 1];

	// 랜덤 숫자 맞큼 넣을 것
	int ran = rand() % (STR_SIZE + 1);

	// 램덤 숫자와 현재 위치의 합이 문자열 크기보다 크면...
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

	//int dEneueSize = ringBuffer->DirectEnqueueSize();

	//if (ran < dEneueSize)
	//{
	//	memcpy(ringBuffer->GetRearBufferPtr(), buffer, ran);

	//	ringBuffer->MoveRear(ran);

	//	cur = (cur + ran) % STR_SIZE;

	//	return ran;
	//}
	//else
	//{
	//	memcpy(ringBuffer->GetRearBufferPtr(), buffer, dEneueSize);

	//	ringBuffer->MoveRear(dEneueSize);

	//	if (ringBuffer->IsFrontZero())
	//	{
	//		cur = (cur + dEneueSize) % (STR_SIZE + 1);
	//	}
	//	else
	//	{
	//		cur = (cur + dEneueSize) % STR_SIZE;
	//	}

	//	return dEneueSize;
	//}

	int size = ringBuffer->Enqueue(buffer, ran);

	cur = (cur + size) % STR_SIZE;

	return size;
}
