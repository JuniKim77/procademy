#include "RingBuffer.h"
#include <cstring>

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
	mBuffer = new char[iBufferSize + 1];
}

void RingBuffer::Resize(int size)
{
	if (size > mCapacity)
	{
		char* temp = new char[size + 1];
		if (mRear >= mFront)
		{
			memcpy(temp, mBuffer, mCapacity);
			delete[] mBuffer;
			mBuffer = temp;
			mCapacity = size;
		}
		else
		{

		}

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
		return mCapacity - mRear - 1;
	}
}

int RingBuffer::DirectDequeueSize(void)
{
	if (mRear >= mFront)
	{
		return mRear 
	}
	return 0;
}

int RingBuffer::Enqueue(char* chpData, int iSize)
{
	if (iSize > GetFreeSize())
	{
		return 0;
	}

	if (mRear >= mFront)
	{
		if (mFront == 0)
	}

	return 0;
}

int RingBuffer::Dequeue(char* chpDest, int iSize)
{
	return 0;
}

int RingBuffer::Peek(char* chpDest, int iSize)
{
	return 0;
}

void RingBuffer::MoveRear(int iSize)
{
}

int RingBuffer::MoveFront(int iSize)
{
	return 0;
}

void RingBuffer::ClearBuffer(void)
{
	mFront = mRear;
}

char* RingBuffer::GetFrontBufferPtr(void)
{
	return nullptr;
}

char* RingBuffer::GetRearBufferPtr(void)
{
	return nullptr;
}


