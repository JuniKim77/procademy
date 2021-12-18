#include "RingBuffer.h"
#include <cstring>
#include <stdio.h>
#include "CLogger.h"
#include "CProfiler.h"

RingBuffer::RingBuffer()
	: RingBuffer(DEFAULT_SIZE)
{
	InitializeSRWLock(&mSrwLock);
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
	mBuffer = new char[(long long)iBufferSize + 1];
	memset(mBuffer, 0, (long long)iBufferSize + 1);
}

void RingBuffer::Resize(int size)
{
	if (size <= mCapacity)
		return;

	char* temp = new char[(long long)size + 1];
	if (mRear >= mFront)
	{
		memcpy(temp, mBuffer, (long long)mCapacity + 1);
		delete[] mBuffer;
		mBuffer = temp;
		mCapacity = size;
	}
	else
	{
		int poss = DirectDequeueSize();
		memcpy(temp, mBuffer + mFront, poss);
		memcpy(temp + poss, mBuffer, mRear);
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
	int rear = mRear;
	int front = mFront;

	if (rear >= front)
		return rear - front;
	else // f 바로 뒤는 넣을 수 없다.
		return mCapacity - (front - rear - 1);
}

int RingBuffer::GetFreeSize(void)
{
	return mCapacity - GetUseSize();
}

int RingBuffer::DirectEnqueueSize(void)
{
	int rear = mRear;
	int front = mFront;

	// Rear가 움직인다...
	// 역방향의 경우, f의 뒤는 항시 비어야 하므로, -1...
	if (rear < front)
	{
		return front - rear - 1;
	}
	// 순방향의 경우, mFront가 0인 경우, 마지막 칸을 비워 둬야 한다...
	if (front == 0)
	{
		return mCapacity - rear;
	}
	else
	{
		return mCapacity - rear + 1;
	}
}

int RingBuffer::DirectDequeueSize(void)
{
	int rear = mRear;
	int front = mFront;

	// Front가 움직여 나간다...
	// 순방향 경우 인덱스 차 반환
	if (rear >= front)
	{
		return rear - front;
	}

	// 끝까지 다 쓸 있어서 +1
	return mCapacity - front + 1;
}

int RingBuffer::Enqueue(char* chpData, int iSize)
{
	if (iSize <= 0)
	{
		return 0;
	}

	int front = mFront;
	int freeSize;
	int rear = mRear;
	int capacity = mCapacity;
	char* buf = mBuffer;

	if (rear >= front)
	{
		freeSize = capacity - (rear - front);
	}
	else
	{
		freeSize = (front - rear - 1);
	}

	// iSize = iSize > freeSize ? freeSize : iSize;

	if (iSize > freeSize)
	{
		if (mLogMode)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Enqueue size over");
		}
		
		iSize = freeSize;
	}

	if (rear + iSize > capacity + 1)
	{
		int possibleToEnd = capacity - rear + 1;

		memcpy(buf + rear, chpData, possibleToEnd);
		memcpy(buf, chpData + possibleToEnd, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(buf + rear, chpData, iSize);
	}

	if (rear + iSize > capacity + 1)
	{
		mRear = (rear + iSize) - (capacity + 1);
	}
	else
	{
		mRear = (rear + iSize);
	}

	return iSize;
}

int RingBuffer::Dequeue(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	int useSize;
	int rear = mRear;
	int front = mFront;
	int capacity = mCapacity;
	char* buf = mBuffer;

	if (rear >= front)
	{
		useSize = rear - front;
	}
	else
	{
		useSize = capacity - (front - rear - 1);
	}

	//iSize = iSize > useSize ? useSize : iSize;

	if (iSize > useSize)
	{
		if (mLogMode)
		{
			CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Dequeue size over");
		}

		iSize = useSize;
	}

	if (front + iSize >= capacity + 1)
	{
		int possibleToEnd = capacity + 1 - front;

		memcpy(chpDest, buf + front, possibleToEnd);
		memcpy(chpDest + possibleToEnd, buf, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(chpDest, buf + front, iSize);
	}

	if (front + iSize > capacity + 1)
	{
		mFront = (front + iSize) - (capacity + 1);
	}
	else
	{
		mFront = (front + iSize);
	}

	return iSize;
}

int RingBuffer::Peek(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	int useSize;
	int rear = mRear;
	int front = mFront;
	int capacity = mCapacity;
	char* buf = mBuffer;

	if (rear >= front)
	{
		useSize = rear - front;
	}
	else
	{
		useSize = capacity - (front - rear - 1);
	}

	iSize = iSize > useSize ? useSize : iSize;

	if (front + iSize >= capacity + 1)
	{
		int possibleToEnd = capacity + 1 - front;

		memcpy(chpDest, buf + front, possibleToEnd);
		memcpy(chpDest + possibleToEnd, buf, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(chpDest, buf + front, iSize);
	}

	return iSize;
}

bool RingBuffer::MoveRear(int iSize)
{
	if (iSize > GetFreeSize() || iSize <= 0)
	{
		return false;
	}

	int rear = mRear;
	int capacity = mCapacity;

	if (rear + iSize > capacity + 1)
	{
		mRear = (rear + iSize) - (capacity + 1);
	}
	else
	{
		mRear = (rear + iSize);
	}

	//mRear = (mRear + iSize) % (mCapacity + 1);

	return true;
}

bool RingBuffer::MoveFront(int iSize)
{
	if (iSize > GetUseSize() || iSize <= 0)
	{
		return false;
	}

	int front = mFront;
	int capacity = mCapacity;

	if (front + iSize > capacity + 1)
	{
		mFront = (front + iSize) - (capacity + 1);
	}
	else
	{
		mFront = (front + iSize);
	}

	//mFront = (mFront + iSize) % (mCapacity + 1);

	return true;
}

void RingBuffer::ClearBuffer(void)
{
	mFront = mRear;
}

char* RingBuffer::GetFrontBufferPtr(void)
{
	return mBuffer + mFront;
}

char* RingBuffer::GetRearBufferPtr(void)
{
	return mBuffer + mRear;
}

void RingBuffer::printInfo()
{
	char buffer[BUFFER_SIZE] = { 0, };

	if (mRear >= mFront)
	{
		memcpy(buffer, mBuffer + mFront, (long long)mRear - mFront);
	}
	else
	{
		int poss = DirectDequeueSize();
		memcpy(buffer, mBuffer + mFront, poss);
		memcpy(buffer + poss, mBuffer, mRear);
	}

	printf("Size: %d, F: %d, R: %d, Buffer: %s\n", GetUseSize(), mFront, mRear, buffer);
}

void RingBuffer::Lock(bool readonly)
{
	if (readonly)
	{
		AcquireSRWLockShared(&mSrwLock);
	}
	else
	{
		AcquireSRWLockExclusive(&mSrwLock);
	}
}

void RingBuffer::Unlock(bool readonly)
{
	if (readonly)
	{
		ReleaseSRWLockShared(&mSrwLock);
	}
	else
	{
		ReleaseSRWLockExclusive(&mSrwLock);
	}
}
