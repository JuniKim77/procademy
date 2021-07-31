#include "RingBuffer.h"
#include <cstring>
#include <stdio.h>

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
	mBuffer = new char[iBufferSize + 1];
	// memset(mBuffer, 0, iBufferSize + 1);
}

void RingBuffer::Resize(int size)
{
	if (size <= mCapacity)
		return;

	char* temp = new char[size + 1];
	if (mRear >= mFront)
	{
		memcpy(temp, mBuffer, mCapacity + 1);
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
	// Rear�� �����δ�...
	// �������� ���, f�� �ڴ� �׽� ���� �ϹǷ�, -1...
	if (mRear < mFront)
	{
		return mFront - mRear - 1;
	}
	// �������� ���, mFront�� 0�� ���, ������ ĭ�� ��� �־� �Ѵ�...
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
	// Front�� ������ ������...
	// ������ ��� �ε��� �� ��ȯ
	if (mRear >= mFront)
	{
		return mRear - mFront;
	}

	// ��� �����̶� +1����...
	return mCapacity - mFront + 1;
}

int RingBuffer::Enqueue(char* chpData, int iSize)
{
	if (iSize <= 0)
	{
		return 0;
	}

	if (iSize > GetFreeSize())
	{
		return Enqueue(chpData, GetFreeSize());
	}

	if (mRear >= mFront)
	{
		int possibleToEnd = DirectEnqueueSize();

		if (iSize <= possibleToEnd)
		{
			memcpy(mBuffer + mRear, chpData, iSize);
			mRear += iSize;

			return iSize;
		}

		int remain = iSize - possibleToEnd;

		memcpy(mBuffer + mRear, chpData, possibleToEnd);
		memcpy(mBuffer, chpData + possibleToEnd, remain);

		mRear = remain;

		return iSize;
	}

	memcpy(mBuffer + mRear, chpData, iSize);

	mRear += iSize;

	return iSize;
}

int RingBuffer::Dequeue(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	if (iSize > GetUseSize())
	{
		return Dequeue(chpDest, GetUseSize());
	}

	if (mFront > mRear)
	{
		int possibleToEnd = DirectDequeueSize();

		if (iSize <= possibleToEnd)
		{
			memcpy(chpDest, mBuffer + mFront, iSize);
			mFront += iSize;

			return iSize;
		}

		int remain = iSize - possibleToEnd;

		memcpy(chpDest, mBuffer + mFront, possibleToEnd);
		memcpy(chpDest + possibleToEnd, mBuffer, remain);

		mFront = remain;

		return iSize;
	}

	memcpy(chpDest, mBuffer + mFront, iSize);

	mFront += iSize;

	return iSize;
}

int RingBuffer::Peek(char* chpDest, int iSize)
{
	if (iSize <= 0) {
		return 0;
	}

	if (iSize > GetUseSize())
	{
		return Peek(chpDest, GetUseSize());
	}

	if (mFront > mRear)
	{
		int possibleToEnd = DirectDequeueSize();

		if (iSize <= possibleToEnd)
		{
			memcpy(chpDest, mBuffer + mFront, iSize);

			return iSize;
		}

		int remain = iSize - possibleToEnd;

		memcpy(chpDest, mBuffer + mFront, possibleToEnd);
		memcpy(chpDest + possibleToEnd, mBuffer, remain);

		return iSize;
	}

	memcpy(chpDest, mBuffer + mFront, iSize);

	return iSize;
}

bool RingBuffer::MoveRear(int iSize)
{
	if (iSize > GetFreeSize() || iSize <= 0)
	{
		return false;
	}
	// �������� ���...
	if (mRear >= mFront)
	{
		// �ٷ� ���� �� �ִ� ũ��..
		int possibleToEnd = DirectEnqueueSize();

		// �� �ͺ��� ���� �ڷ���� ���ٸ�.. �׳� ����!
		if (iSize <= possibleToEnd)
		{
			// �ε����� ������ �ʰ��ϴ� ��츦 ������...
			if (IsFrontZero())
			{
				mRear += iSize;
			}
			else
			{
				mRear = (mRear + iSize) % (mCapacity + 1);
			}

			return true;
		}

		int remain = iSize - possibleToEnd;

		mRear = remain;

		return true;
	}
	// �������� �׳� ������ ��...
	mRear += iSize;

	return true;
}

bool RingBuffer::MoveFront(int iSize)
{
	if (iSize > GetUseSize() || iSize <= 0)
	{
		return false;
	}
	// �������� ���,,,
	if (mFront > mRear)
	{
		int possibleToEnd = DirectDequeueSize();

		if (iSize <= possibleToEnd)
		{
			mFront = (mFront + iSize) % (mCapacity + 1);

			return true;
		}

		int remain = iSize - possibleToEnd;

		mFront = remain;

		return true;
	}
	// �������̸� �׳� ������ ��...
	mFront += iSize;

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
