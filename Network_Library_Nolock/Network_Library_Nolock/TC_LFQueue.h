#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

template <typename DATA>
class TC_LFQueue
{
private:
	int mSize = 0;

	struct Node
	{
		DATA data;
		Node* next;
	};

	struct t_Top
	{
		Node* ptr_node = nullptr;
		LONG64 counter = -9999;
	};

	alignas(16) t_Top mHead;        // 시작노드를 포인트한다.
	alignas(16) t_Top mTail;        // 마지막노드를 포인트한다.
	//Node* mTail = nullptr;        // 마지막노드를 포인트한다.
	procademy::TC_LFObjectPool<Node> mMemoryPool;

public:
	enum {
		LOGIC_DEQUEUE = 10000,
		LOGIC_ENQUEUE = 20000
	};
	TC_LFQueue();
	~TC_LFQueue();
	void Enqueue(DATA data);
	bool Dequeue(DATA* data);
	DWORD Peek(DATA arr[]);
	bool IsEmpty() { return mSize == 0; }
	DWORD GetSize() { return mSize; }
	DWORD GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
	DWORD GetPoolSize() { return mMemoryPool.GetSize(); }
	void linkCheck(int size);
	void Log(int logicId, t_Top snap_top, Node* next, bool isHead = false);
	void Clear();

private:
	void MoveTail(int logicId, t_Top* snap, Node** next);
};

template<typename DATA>
inline TC_LFQueue<DATA>::TC_LFQueue()
{
	Node* dummy = mMemoryPool.Alloc();

	dummy->next = nullptr;
	mHead.ptr_node = dummy;
	mHead.counter = 0;
	mTail.ptr_node = dummy;
	mTail.counter = 0;
}

template<typename DATA>
inline TC_LFQueue<DATA>::~TC_LFQueue()
{
	Node* node = mHead.ptr_node;
	int count = 0;
	while (node != nullptr)
	{
		Node* temp = node;

		node = node->next;
		count++;
		mMemoryPool.Free(temp);
	}
}

template<typename DATA>
inline void TC_LFQueue<DATA>::Enqueue(DATA data)
{
	alignas(16) t_Top top;
	Node* node = mMemoryPool.Alloc();
	Node* next;

	node->data = data;
	node->next = nullptr;

	//Log(LOGIC_ENQUEUE, top, node);

	while (1)
	{
		top.counter = mTail.counter;
		top.ptr_node = mTail.ptr_node;
		next = top.ptr_node->next;

		if (next == nullptr)
		{
			Node* snap_next = mTail.ptr_node;
			if (InterlockedCompareExchangePointer((PVOID*)&top.ptr_node->next, node, nullptr) == nullptr)
			{
				if (snap_next == node)
				{
					int test = 0;
				}
				InterlockedIncrement((DWORD*)&mSize);
				//Log(LOGIC_ENQUEUE + 50, top, next);

				MoveTail(LOGIC_ENQUEUE + 70, &top, &next);

				break;
			}
		}
		else
		{
			MoveTail(LOGIC_ENQUEUE + 20, &top, &next);
		}
	}
}

template<typename DATA>
inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
{
	alignas(16) t_Top top;
	alignas(16) t_Top tail;

	InterlockedDecrement((DWORD*)&mSize);

	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);

		return false;
	}

	Node* next;
	Node* tail_next;
	DATA snap_data;

	while (1)
	{
		top.counter = mHead.counter;
		top.ptr_node = mHead.ptr_node;
		next = top.ptr_node->next;

		tail.counter = mTail.counter;
		tail.ptr_node = mTail.ptr_node;
		tail_next = tail.ptr_node->next;

		if (next != nullptr)
		{
			if (top.ptr_node == tail.ptr_node)
			{
				MoveTail(LOGIC_ENQUEUE + 20, &tail, &tail_next);
			}
			else
			{
				snap_data = next->data;
				if (InterlockedCompareExchange128((LONG64*)&mHead, top.counter + 1, (LONG64)next, (LONG64*)&top))
				{
					//Log(LOGIC_DEQUEUE + 50, top, next, true);
					break;
				}
			}
		}
	}

	*data = snap_data;

	mMemoryPool.Free(top.ptr_node);

	return true;
}

template<typename DATA>
inline DWORD TC_LFQueue<DATA>::Peek(DATA arr[])
{
	DWORD i;
	Node* pHead = mHead.ptr_node->next;

	for (i = 0; i < 100; ++i)
	{
		if (pHead == nullptr)
		{
			return i;
		}

		arr[i] = pHead->data;
		pHead = pHead->next;
	}

	return i;
}

template<typename DATA>
inline void TC_LFQueue<DATA>::linkCheck(int size)
{
	Node* node = mHead.ptr_node->next;

	int count = 0;

	while (node != nullptr)
	{
		count++;
		node = node->next;
	}

	if (count == size)
	{
		wprintf_s(L"Enqueued Successfully\n=====================\n");
	}
}

template<typename DATA>
inline void TC_LFQueue<DATA>::Log(int logicId, t_Top snap_top, Node* next, bool isHead)
{
	if (isHead)
	{
		_Log(logicId, GetCurrentThreadId(), mSize, mHead.counter, snap_top.counter, mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next, snap_top.ptr_node, next);
	}
	else
	{
		_Log(logicId, GetCurrentThreadId(), mSize, mTail.counter, snap_top.counter, mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next, snap_top.ptr_node, next);
	}
	
}

template<typename DATA>
inline void TC_LFQueue<DATA>::Clear()
{
	int size = mSize;
	DATA temp;
	for (int i = 0; i < size; ++i)
	{
		Dequeue(&temp);
	}
}

template<typename DATA>
inline void TC_LFQueue<DATA>::MoveTail(int logicId, t_Top* snap, Node** next)
{
	snap->counter = mTail.counter;
	snap->ptr_node = mTail.ptr_node;
	*next = snap->ptr_node->next;

	if (*next != nullptr)
	{
		// Log(logicId - 10, *snap, *next);
		if (InterlockedCompareExchange128((LONG64*)&mTail, snap->counter + 1, (LONG64)*next, (LONG64*)snap) == 0)
		{
			//Log(logicId, snap, nullptr);
		}
		else
		{
			//Log(logicId, *snap, *next);
		}
	}
}