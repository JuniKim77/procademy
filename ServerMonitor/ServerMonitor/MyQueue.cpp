#include "MyQueue.h"

MyQueue::MyQueue(int capacity)
	: mCapacity(capacity)
{
	mBuffer = new int[mCapacity];
}

MyQueue::~MyQueue()
{
	if (mBuffer != nullptr)
		delete[] mBuffer;
}

void MyQueue::enqueue(int value)
{
	mBuffer[mRear] = value;

	mRear = (mRear + 1) % QUEUE_SIZE;

	if (mFront == mRear)
	{
		mFront = (mFront + 1) % QUEUE_SIZE;
	}
}

void MyQueue::popqueue()
{
	mFront = (mFront + 1) % mCapacity;
}

int MyQueue::peakqueue(int pos)
{
	return mBuffer[pos];
}
