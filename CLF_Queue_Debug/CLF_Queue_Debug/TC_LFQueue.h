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
	void* snapNode;
	void* snapNext;
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
	void* tailNext = nullptr,
	void* snapNode = nullptr,
	void* snapNext = nullptr
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
	g_debugs[index].snapNode = snapNode;
	g_debugs[index].snapNext = snapNext;
}

USHORT finder_log()
{
	for (USHORT i = 0; i <= g_debug_index; ++i)
	{
		if (g_debugs[i].logicId == 20060 && 
			g_debugs[i].head == g_debugs[i].tail &&
			g_debugs[i].size == 1)
		{
			return i;
		}
	}

	return USHRT_MAX;
}

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
		LONG64 counter = 0;
	};

	alignas(16) t_Top mHead;        // 시작노드를 포인트한다.
	Node* mTail = nullptr;        // 마지막노드를 포인트한다.
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
    bool IsEmpty() { return mSize == 0; }
    DWORD GetSize() { return mSize; }
    DWORD GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
    DWORD GetPoolSize() { return mMemoryPool.GetSize(); }
	void linkCheck(int size);
	void Log(int logicId, Node* node, Node* next);
};

template<typename DATA>
inline TC_LFQueue<DATA>::TC_LFQueue()
{
    Node* dummy = mMemoryPool.Alloc();

    dummy->next = nullptr;
	mHead.ptr_node = dummy;
    mTail = dummy;
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
	//Log(LOGIC_ENQUEUE, nullptr, nullptr);
	//InterlockedIncrement((DWORD*)&mSize);
	//Log(LOGIC_ENQUEUE + 10, nullptr, nullptr);
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
			/*do
			{
				tail = mTail;
				next = tail->next;
			} while (tail != mTail);*/
		} while (next != nullptr);

		Log(LOGIC_ENQUEUE + 20, tail, next);
		if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, next) == next)
		{
			while (1)
			{
				Log(LOGIC_ENQUEUE + 30, tail, next);
				if (InterlockedCompareExchangePointer((PVOID*)&mTail, node, tail) == tail)
				{
					Log(LOGIC_ENQUEUE + 40, tail, next);
					InterlockedIncrement((DWORD*)&mSize);
					Log(LOGIC_ENQUEUE + 60, tail, next);

					break;
				}
				else
				{
					Log(LOGIC_ENQUEUE + 50, tail, next);
					tail = mTail;
					next = tail->next;
				}
			}
			//mTail = mTail->next;

			break;
		}
	}
}

template<typename DATA>
inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
{
	alignas(16) t_Top top;
	//Log(LOGIC_DEQUEUE, nullptr, nullptr);
	InterlockedDecrement((DWORD*)&mSize);
	//Log(LOGIC_DEQUEUE + 10, nullptr, nullptr);
	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);

		return false;
	}
	Log(LOGIC_DEQUEUE + 10, nullptr, nullptr);

	Node* next;
	USHORT idx;

	do
	{
		/*do
		{
			top = mHead;
			next = top->next;
		} while (next != mHead->next);
		
		if (next == nullptr)
		{
			idx = finder_log();
			CRASH();
		}*/
		do
		{
			top.ptr_node = mHead.ptr_node;
			top.counter = mHead.counter;
			next = top.ptr_node->next;
		} while (top.ptr_node != mHead.ptr_node);

		Log(LOGIC_DEQUEUE + 20, top.ptr_node, next);
	} while (InterlockedCompareExchange128((LONG64*)&mHead, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);
	Log(LOGIC_DEQUEUE + 30, top.ptr_node, next);

	*data = next->data;
	mMemoryPool.Free(top.ptr_node);

	return true;
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
inline void TC_LFQueue<DATA>::Log(int logicId, Node* node, Node* next)
{
	_Log(logicId, GetCurrentThreadId(), mSize, mHead.ptr_node, mHead.ptr_node->next, mTail, mTail->next, node, next);
}
