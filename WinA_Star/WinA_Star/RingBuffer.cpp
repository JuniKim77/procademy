#include "RingBuffer.h"
#include <cstring>
#include <stdio.h>
#include "AStar.h"

RingBuffer::RingBuffer()
	: RingBuffer(DEFAULT_SIZE)
{
}

RingBuffer::~RingBuffer()
{
	if (mBuffer != nullptr)
		delete[] mBuffer;
}

RingBuffer::RingBuffer(int iBufferSize)
	: mFront(0)
	, mRear(0)
	, mCapacity(iBufferSize)
	, mBuffer(nullptr)
{
	mBuffer = new Node*[iBufferSize + 1];
	memset(mBuffer, 0, sizeof(Node*) * (iBufferSize + 1));
}

void RingBuffer::Resize(int size)
{
	if (size <= mCapacity)
		return;

	Node** temp = new Node*[size + 1];
	memset(mBuffer, 0, sizeof(Node*) * (size + 1));

	if (mRear >= mFront)
	{
		memcpy(temp, mBuffer, mCapacity + 1);
		delete[] mBuffer;
		mBuffer = temp;
		mCapacity = size;
	}
	else ///
	{
		int poss = DirectDequeueSize();
		memcpy(temp, mBuffer + mFront, poss * sizeof(Node*));
		memcpy(temp + poss * sizeof(Node*), mBuffer, mRear * sizeof(Node*));
		delete[] mBuffer;
		mBuffer = temp;
		int useSize = GetUseSize();

		mFront = 0;
		mRear = useSize;
		mCapacity = size;
	}
}

int RingBuffer::GetBufferSize(void)
{
	return mCapacity;
}

int RingBuffer::GetUseSize(void)
{
	if (mRear >= mFront)
		return mRear - mFront;
	else
		return mCapacity - (mFront - mRear) + 1;
}

int RingBuffer::GetFreeSize(void)
{
	return mCapacity - GetUseSize();
}

int RingBuffer::DirectEnqueueSize(void)
{
	if (mRear < mFront)
	{
		return mFront - mRear - 1;
	}

	if (mFront == 0)
	{
		return mCapacity - mRear;
	}
	else
	{
		return mCapacity - mRear + 1;
	}
}

int RingBuffer::DirectDequeueSize(void)
{
	if (mRear >= mFront)
	{
		return mRear - mFront;
	}

	return mCapacity - mFront + 1;
}

void RingBuffer::Enqueue(Node* node)
{
	if (GetFreeSize() == 0)
	{
		Resize(mCapacity * 2);
	}

	mBuffer[mRear] = node;

	mRear = (mRear + 1) % mCapacity;
}

void RingBuffer::Dequeue(Node** dest)
{
	if (GetUseSize() == 0)
	{
		return;
	}

	*dest = mBuffer[mFront];

	mFront = (mFront + 1) % mCapacity;
}

void RingBuffer::ClearBuffer(void)
{
	mFront = mRear;
}

void RingBuffer::printInfo()
{
	char buffer[BUFFER_SIZE] = { 0, };

	if (mRear >= mFront)
	{
		memcpy(buffer, mBuffer + mFront, mRear - mFront);
	}
	else
	{
		int poss = DirectDequeueSize();
		memcpy(buffer, mBuffer + mFront, poss);
		memcpy(buffer + poss, mBuffer, mRear - 1);
	}

	printf("Size: %d, F: %d, R: %d, Buffer: %s\n", GetUseSize(), mFront, mRear, buffer);
}


