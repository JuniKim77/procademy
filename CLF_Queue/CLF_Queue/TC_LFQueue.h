#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

template <typename DATA>
class TC_LFQueue
{
public:
    TC_LFQueue();
    ~TC_LFQueue();
    void Enqueue(DATA data);
    bool Dequeue(DATA* data);
    bool IsEmpty() { return mSize == 0; }
    DWORD GetSize() { return mSize; }
    DWORD GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
    DWORD GetPoolSize() { return mMemoryPool.GetSize(); }
	void linkCheck(int size);

private:
    int mSize = 0;

    struct Node
    {
        DATA data;
        Node* next;
    };

    Node* mHead = nullptr;        // 시작노드를 포인트한다.
    Node* mTail = nullptr;        // 마지막노드를 포인트한다.
    procademy::TC_LFObjectPool<Node> mMemoryPool;
};

template<typename DATA>
inline TC_LFQueue<DATA>::TC_LFQueue()
{
    Node* dummy = mMemoryPool.Alloc();

    dummy->next = nullptr;
    mHead = dummy;
    mTail = dummy;
}

template<typename DATA>
inline TC_LFQueue<DATA>::~TC_LFQueue()
{
    Node* node = mHead;
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
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);

	(*index)++;

	debug[*index].type = 'E';

	InterlockedIncrement((DWORD*)&mSize);
	debug[*index].size1 = mSize;

	Node* node = mMemoryPool.Alloc();
	Node* tail;
	Node* next;

	node->data = data;
	node->next = nullptr;

	while (1)
	{
		do
		{
			tail = mTail;
			next = tail->next;
		} while (next != nullptr);

		debug[*index].address1 = tail;

		if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, next) == next)
		{
			debug[*index].address2 = mTail;
			if (InterlockedCompareExchangePointer((PVOID*)&mTail, node, tail) == tail)
			{
				
			}
			else
			{
				CRASH();
			}
			debug[*index].address3 = mTail;
			break;
		}
	}
}

template<typename DATA>
inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
{
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);

	(*index)++;

	debug[*index].type = 'D';

	InterlockedDecrement((DWORD*)&mSize);
	debug[*index].size1 = mSize;

	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);

		return false;
	}

	Node* top;
	Node* next;

	do
	{
		top = mHead;
		next = top->next;
		top->data = next->data;
		debug[*index].address1 = top;
	} while (InterlockedCompareExchangePointer((PVOID*)&mHead, next, top) != top);
	debug[*index].size2 = mSize;
	debug[*index].address2 = mHead;

	*data = top->data;
	mMemoryPool.Free(top);

	return true;
}

template<typename DATA>
inline void TC_LFQueue<DATA>::linkCheck(int size)
{
	Node* node = mHead->next;

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
