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

	if (rear >= mFront)
		return rear - mFront;
	else // f 바로 뒤는 넣을 수 없다.
		return mCapacity - (mFront - rear - 1);
}

int RingBuffer::GetFreeSize(void)
{
	return mCapacity - GetUseSize();
}

int RingBuffer::DirectEnqueueSize(void)
{
	// Rear가 움직인다...
	// 역방향의 경우, f의 뒤는 항시 비어야 하므로, -1...
	if (mRear < mFront)
	{
		return mFront - mRear - 1;
	}
	// 순방향의 경우, mFront가 0인 경우, 마지막 칸을 비워 둬야 한다...
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
	// Front가 움직여 나간다...
	// 순방향 경우 인덱스 차 반환
	if (mRear >= mFront)
	{
		return mRear - mFront;
	}

	// 끝까지 다 쓸 있어서 +1
	return mCapacity - mFront + 1;
}

int RingBuffer::Enqueue(char* chpData, int iSize)
{
	if (iSize <= 0)
	{
		return 0;
	}

	int curFront = mFront;
	int freeSize = 0;

	if (mRear >= curFront)
	{
		freeSize = mCapacity - (mRear - curFront);
	}
	else
	{
		freeSize = (curFront - mRear - 1);
	}

	// iSize = iSize > freeSize ? freeSize : iSize;

	if (iSize > freeSize)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Enqueue size over");
		iSize = freeSize;
	}

	if (mRear + iSize > mCapacity + 1)
	{
		int possibleToEnd = mCapacity - mRear + 1;

		memcpy(mBuffer + mRear, chpData, possibleToEnd);
		memcpy(mBuffer, chpData + possibleToEnd, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(mBuffer + mRear, chpData, iSize);
	}

	mRear = (mRear + iSize) % (mCapacity + 1);

	return iSize;
}

int RingBuffer::Dequeue(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	int useSize;
	int curRear = mRear;

	if (curRear >= mFront)
	{
		useSize = curRear - mFront;
	}
	else
	{
		useSize = mCapacity - (mFront - curRear - 1);
	}

	//iSize = iSize > useSize ? useSize : iSize;

	if (iSize > useSize)
	{
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Dequeue size over");
		iSize = useSize;
	}

	if (mFront + iSize >= mCapacity + 1)
	{
		int possibleToEnd = mCapacity + 1 - mFront;

		memcpy(chpDest, mBuffer + mFront, possibleToEnd);
		memcpy(chpDest + possibleToEnd, mBuffer, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(chpDest, mBuffer + mFront, iSize);
	}

	mFront = (mFront + iSize) % (mCapacity + 1);

	return iSize;
}

int RingBuffer::Peek(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	int useSize;
	int curRear = mRear;

	if (curRear >= mFront)
	{
		useSize = curRear - mFront;
	}
	else
	{
		useSize = mCapacity - (mFront - curRear - 1);
	}

	iSize = iSize > useSize ? useSize : iSize;

	if (mFront + iSize >= mCapacity + 1)
	{
		int possibleToEnd = mCapacity + 1 - mFront;

		memcpy(chpDest, mBuffer + mFront, possibleToEnd);
		memcpy(chpDest + possibleToEnd, mBuffer, (long long)iSize - possibleToEnd);
	}
	else
	{
		memcpy(chpDest, mBuffer + mFront, iSize);
	}

	return iSize;
}

bool RingBuffer::MoveRear(int iSize)
{
	if (iSize > GetFreeSize() || iSize <= 0)
	{
		return false;
	}

	mRear = (mRear + iSize) % (mCapacity + 1);

	return true;
}

bool RingBuffer::MoveFront(int iSize)
{
	if (iSize > GetUseSize() || iSize <= 0)
	{
		return false;
	}

	mFront = (mFront + iSize) % (mCapacity + 1);

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
