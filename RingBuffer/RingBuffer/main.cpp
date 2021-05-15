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

	RingBuffer ringBuffer(BUFFER_SIZE);
	srand(10);

	while (1)
	{
		int ran = enqueueProc(&ringBuffer);

		//printf("After Enqueue::: Ran: %d\n", ran);
		//ringBuffer.printInfo();

		ran = dequeueProc(&ringBuffer);

		//printf("After Enqueue::: Ran: %d\n", ran);
		//ringBuffer.printInfo();
	}

	return 0;
}

int dequeueProc(RingBuffer* ringBuffer)
{
	char buffer[STR_SIZE + 1];

	int ran = rand() % (STR_SIZE + 1);

	if (ringBuffer->Dequeue(buffer, ran) == true)
	{
		buffer[ran] = '\0';
		printf(buffer);
		//printf("SUCCESS::  ");
	}

	return ran;
}

int enqueueProc(RingBuffer* ringBuffer)
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

	if (ringBuffer->Enqueue(buffer, ran) == true)
	{
		cur = (cur + ran) % STR_SIZE;
		//printf("SUCCESS::  ");
	}

	return ran;
}
