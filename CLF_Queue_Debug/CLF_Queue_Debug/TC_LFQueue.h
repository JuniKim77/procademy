#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

struct st_DEBUG
{
	int logicId;
	DWORD threadId;
	void* head;
	void* headNext;
	void* tail;
	void* tailNext;
	int size;
};

USHORT g_debug_index = 0;
st_DEBUG g_debugs[USHRT_MAX + 1] = { 0, };

void _Log(
	int logicId = -9999,
	DWORD threadId = 0,
	int size = 9999,
	void* head = nullptr,
	void* headNext = nullptr,
	void* tail = nullptr,
	void* tailNext = nullptr
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debug_index);

	g_debugs[index].logicId = logicId;
	g_debugs[index].threadId = threadId;
	g_debugs[index].size = size;
	g_debugs[index].head = head;
	g_debugs[index].headNext = headNext;
	g_debugs[index].tail = tail;
	g_debugs[index].tailNext = tailNext;
}

USHORT finder_log()
{
	for (USHORT i = g_debug_index; i >= 0; --i)
	{
		if (g_debugs[i].logicId == 20040 && g_debugs[i].tailNext != nullptr)
		{
			return i;
		}

		if (g_debugs[i].logicId == 20050 && g_debugs[i].tailNext != nullptr)
		{
			return i;
		}
	}

	return USHRT_MAX;
}

template <typename DATA>
class TC_LFQueue
{
public:
	enum {
		LOGIC_DEQUEUE = 10000,
		LOGIC_ENQUEUE = 20000
	};
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
	_Log(LOGIC_ENQUEUE, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
	InterlockedIncrement((DWORD*)&mSize);
	_Log(LOGIC_ENQUEUE + 10, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
	Node* node = mMemoryPool.Alloc();
	Node* tail;
	Node* next;
	USHORT idx;

	node->data = data;
	node->next = nullptr;

	while (1)
	{
		do
		{
			tail = mTail;
			next = tail->next;
		} while (next != nullptr);
		_Log(LOGIC_ENQUEUE + 20, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
		if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, next) == next)
		{
			_Log(LOGIC_ENQUEUE + 30, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
			if (InterlockedCompareExchangePointer((PVOID*)&mTail, node, tail) == tail)
			{
				_Log(LOGIC_ENQUEUE + 40, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
			}
			else
			{
				_Log(LOGIC_ENQUEUE + 50, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
				idx = finder_log();
				CRASH();
			}
			//mTail = mTail->next;

			break;
		}
	}
}

template<typename DATA>
inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
{
	_Log(LOGIC_DEQUEUE, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
	InterlockedDecrement((DWORD*)&mSize);
	_Log(LOGIC_DEQUEUE + 10, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);

		return false;
	}

	Node* top;
	Node* next;
	USHORT idx;

	do
	{
		top = mHead;
		next = top->next;
		if (next == nullptr)
		{
			idx = finder_log();
			CRASH();
		}
		top->data = next->data;
		_Log(LOGIC_DEQUEUE + 20, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);
	} while (InterlockedCompareExchangePointer((PVOID*)&mHead, next, top) != top);
	_Log(LOGIC_DEQUEUE + 30, GetCurrentThreadId(), mSize, mHead, mHead->next, mTail, mTail->next);

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
