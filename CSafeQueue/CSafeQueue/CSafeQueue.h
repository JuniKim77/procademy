#pragma once
#include "CLogger.h"

//struct ringbufDebug
//{
//	char* pRear;
//	char* pFront;
//	char* pBuf;
//	int mRear;
//	int mFront;
//	int mCapa;
//	ULONG len1;
//	ULONG len2;
//};
//
//USHORT g_debugIdx = 0;
//ringbufDebug g_packetDebugs[USHRT_MAX + 1] = { 0, };

namespace procademy
{
	template <typename DATA>
	class CSafeQueue
	{
	private:
		enum {
			DEFAULT_SIZE = 100
		};

	public:
		CSafeQueue();
		~CSafeQueue();
		CSafeQueue(int iBufferSize);

		int		GetCapacity(void) { return mCapacity; }

		int		GetUseSize(void);

		bool	IsEmpty() { return mFront == mRear; }

		bool	IsFull(void);

		bool	Enqueue(DATA data);

		DATA	Dequeue();

		DWORD	Peek(DATA arr[], DWORD size);

		void	SetLogMode(bool mode) { mLogMode = mode; }

	public:
		int mFront;
		int mRear;
		DATA* mBuffer;
		int mCapacity;
		bool mLogMode = false;
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
		int front = mFront;

		if (rear >= front)
			return rear - front;
		else // f 바로 뒤는 넣을 수 없다.
			return mCapacity - (front - rear - 1);
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
			if (mLogMode)
			{
				CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Queue Is Full");
			}

			return false;
		}

		int rear = mRear;

		mBuffer[rear++] = data;

		if (rear > mCapacity)
		{
			mRear = 0;
		}
		else
		{
			mRear = rear;
		}

		return true;
	}
	template<typename DATA>
	inline DATA CSafeQueue<DATA>::Dequeue()
	{
		if (IsEmpty())
		{
			if (mLogMode)
			{
				CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Queue Is Empty");
			}

			return NULL;
		}

		int front = mFront;

		DATA ret = mBuffer[front++];

		if (front > mCapacity)
		{
			mFront = 0;
		}
		else
		{
			mFront = front;
		}	

		return ret;
	}
	template<typename DATA>
	inline DWORD CSafeQueue<DATA>::Peek(DATA arr[], DWORD size)
	{
		DWORD i;
		int front = mFront;
		int rear = mRear;

		for (i = 0; i < size; ++i)
		{
			arr[i] = mBuffer[front++];

			if (front > mCapacity)
			{
				front = 0;
			}

			if (rear == front)
			{
				i++;
				break;
			}
		}

		return i;
	}
}
