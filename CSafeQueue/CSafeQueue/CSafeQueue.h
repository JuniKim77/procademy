#pragma once
#include "CLogger.h"

namespace procademy
{
	template <typename DATA>
	class CSafeQueue
	{
	private:
		enum {
			DEFAULT_SIZE = 1000
		};

	public:
		CSafeQueue();
		~CSafeQueue();
		CSafeQueue(int iBufferSize);

		int GetCapacity(void) { return mCapacity; }

		int GetUseSize(void);

		bool IsEmpty() { return mFront == mRear; }

		bool IsFull(void);

		bool Enqueue(DATA data);

		DATA Dequeue();

	private:
		int mFront;
		int mRear;
		DATA* mBuffer;
		int mCapacity;
	};
	template<typename DATA>
	inline CSafeQueue<DATA>::CSafeQueue()
		: CSafeQueue(DEFAULT_SIZE)
	{
	}
	template<typename DATA>
	inline CSafeQueue<DATA>::~CSafeQueue()
	{
		delete[] mBuffer;
	}
	template<typename DATA>
	inline CSafeQueue<DATA>::CSafeQueue(int iBufferSize)
		: mFront(0)
		, mRear(0)
		, mCapacity(iBufferSize)
		, mBuffer(nullptr)
	{
		mBuffer = new DATA[(long long)iBufferSize + 1];
		memset(mBuffer, 0, sizeof(DATA) * ((long long)iBufferSize + 1));
	}
	template<typename DATA>
	inline int CSafeQueue<DATA>::GetUseSize(void)
	{
		int rear = mRear;

		if (rear >= mFront)
			return rear - mFront;
		else // f 바로 뒤는 넣을 수 없다.
			return mCapacity - (mFront - rear - 1);
	}
	template<typename DATA>
	inline bool CSafeQueue<DATA>::IsFull(void)
	{
		int rear = mRear;
		int front = mFront;

		if (front < rear)
		{
			return (rear - front) == mCapacity;
		}
		else
		{
			return (rear + 1) == front;
		}
	}
	template<typename DATA>
	inline bool CSafeQueue<DATA>::Enqueue(DATA data)
	{
		if (IsFull())
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Queue Is Full");

			return false;
		}

		mBuffer[mRear] = data;

		if ((mRear + 1) == mCapacity)
		{
			mRear = 0;
		}
		else
		{
			mRear++;
		}

		return true;
	}
	template<typename DATA>
	inline DATA CSafeQueue<DATA>::Dequeue()
	{
		if (IsEmpty())
		{
			return nullptr;
		}

		int front = mFront;

		if ((mFront + 1) == mCapacity)
		{
			mFront = 0;
		}
		else
		{
			mFront++;
		}

		return mBuffer[front];
	}
}
