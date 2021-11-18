#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

extern bool g_exit;

struct st_DEBUG
{
	int			logicId;
	DWORD		threadId;
	void*		head;
	void*		headNext;
	void*		tail;
	void*		tailNext;
	void*		snapNode;
	void*		snapNext;
	LONG64		counter;
	LONG64		snap_counter;
	int			size;
};

USHORT g_debug_index = 0;
st_DEBUG g_debugs[USHRT_MAX + 1] = { 0, };

void _Log(
	int			logicId = -9999,
	DWORD		threadId = 0,
	int			size = 9999,
	LONG64		counter = -9999,
	LONG64		snap_counter = -9999,
	void*		head = nullptr,
	void*		headNext = nullptr,
	void*		tail = nullptr,
	void*		tailNext = nullptr,
	void*		snapNode = nullptr,
	void*		snapNext = nullptr
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debug_index);

	g_debugs[index].logicId = logicId;
	g_debugs[index].threadId = threadId;
	g_debugs[index].size = size;
	g_debugs[index].counter = counter;
	g_debugs[index].snap_counter = snap_counter;
	g_debugs[index].head = head;
	g_debugs[index].headNext = headNext;
	g_debugs[index].tail = tail;
	g_debugs[index].tailNext = tailNext;
	g_debugs[index].snapNode = snapNode;
	g_debugs[index].snapNext = snapNext;
}

USHORT finder_log()
{
	for (USHORT i = g_debug_index; i > 0; --i)
	{
		if (g_debugs[i].logicId == 10050 &&
			g_debugs[i].head == g_debugs[i].headNext)
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
#ifdef VER_CASH_LINE
	alignas(64) t_Top	mHead;        // 시작노드를 포인트한다.
	alignas(64) t_Top	mTail;        // 마지막노드를 포인트한다.
	alignas(64) int		mSize = 0;
#else
	t_Top		mHead;        // 시작노드를 포인트한다.
	t_Top		mTail;        // 마지막노드를 포인트한다.
	int			mSize = 0;
#endif // VER_CASH_LINE	
	procademy::TC_LFObjectPool<Node> mMemoryPool;	

public:
	enum {
		LOGIC_DEQUEUE = 10000,
		LOGIC_ENQUEUE = 20000
	};
	TC_LFQueue();
	~TC_LFQueue();
	void		Enqueue(DATA data);
	bool		Dequeue(DATA* data);
	DWORD Peek(DATA arr[], DWORD size);
	bool		IsEmpty() { return mSize == 0; }
	DWORD		GetSize() { return mSize; }
	DWORD		GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
	DWORD		GetPoolSize() { return mMemoryPool.GetSize(); }
	void		linkCheck(int size);
	void		Log(int logicId, t_Top snap_top, Node* next, bool isHead = false);

private:
	void		MoveTail(int logicId, t_Top* snap);
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
			if (InterlockedCompareExchangePointer((PVOID*)&top.ptr_node->next, node, nullptr) == nullptr)
			{
				InterlockedIncrement((long*)&mSize);
				//Log(LOGIC_ENQUEUE + 50, top, next);

				MoveTail(LOGIC_ENQUEUE + 70, &top);

				break;
			}
		}
		else
		{
			MoveTail(LOGIC_ENQUEUE + 20, &top);
		}
	}
}

template<typename DATA>
inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
{
	alignas(16) t_Top top;
	alignas(16) t_Top tail;

	InterlockedDecrement((long*)&mSize);

	if (mSize < 0)
	{
		InterlockedIncrement((long*)&mSize);

		return false;
	}

	Node* next;

	while (1)
	{
		top.counter = mHead.counter;
		top.ptr_node = mHead.ptr_node;
		next = top.ptr_node->next;

		tail.counter = mTail.counter;
		tail.ptr_node = mTail.ptr_node;

		if (top.ptr_node == tail.ptr_node || next == nullptr)
		{
			MoveTail(LOGIC_ENQUEUE + 20, &tail);
		}
		else
		{
			*data = next->data;
			if (InterlockedCompareExchange128((LONG64*)&mHead, top.counter + 1, (LONG64)next, (LONG64*)&top))
			{
				//Log(LOGIC_DEQUEUE + 50, top, next, true);
				break;
			}
		}
	}

	mMemoryPool.Free(top.ptr_node);

	return true;
}

template<typename DATA>
inline DWORD TC_LFQueue<DATA>::Peek(DATA arr[], DWORD size)
{
	DWORD i;
	Node* pHead = mHead.ptr_node->next;

	for (i = 0; i < size; ++i)
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
inline void TC_LFQueue<DATA>::MoveTail(int logicId, t_Top* snap)
{
	snap->counter = mTail.counter;
	snap->ptr_node = mTail.ptr_node;
	Node* next = snap->ptr_node->next;

	if (next != nullptr)
	{
		//Log(logicId - 10, *snap, *next);
		if (InterlockedCompareExchange128((LONG64*)&mTail, snap->counter + 1, (LONG64)next, (LONG64*)snap) == 0)
		{
			//Log(logicId, snap, nullptr);
		}
		else
		{
			//Log(logicId, *snap, *next);
		}
	}
}