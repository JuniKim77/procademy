#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

struct st_DEBUG
{
	int			logicId;
	DWORD		threadId;
	void*		mTop;
	void*		mTopNext;
	void*		snap_top;
	void*		snap_top_next;
	DWORD		size;
	LONG64		counter;
	LONG64		snap_counter;
};

USHORT g_debug_index = 0;
st_DEBUG g_debugs[USHRT_MAX + 1] = { 0, };

void _Log(
	int			logicId = -9999,
	DWORD		threadId = 0,
	DWORD		size = 9999,
	LONG64		counter = -9999,
	LONG64		snap_counter = -9999,
	void*		top_ptr = nullptr,
	void*		mTopNext = nullptr,
	void*		snap_top = nullptr,
	void*		snap_top_next = nullptr
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debug_index);

	g_debugs[index].logicId = logicId;
	g_debugs[index].threadId = threadId;
	g_debugs[index].size = size;
	g_debugs[index].counter = counter;
	g_debugs[index].snap_counter = snap_counter;
	g_debugs[index].mTop = top_ptr;
	g_debugs[index].mTopNext = mTopNext;
	g_debugs[index].snap_top = snap_top;
	g_debugs[index].snap_top_next = snap_top_next;
}

USHORT finder_log()
{
	for (USHORT i = 0; i < USHRT_MAX; ++i)
	{
		if (g_debugs[i].logicId == 20050 && 
			g_debugs[i].mTop != g_debugs[i].snap_top_next)
		{
			return i;
		}
	}

	return USHRT_MAX;
}

template <typename T>
class TC_LFStack
{
public:
	enum {
		PUSH_DEBUG = 10000,
		POP_DEBUG = 20000
	};

private:
	struct Node
	{
		T data;
		Node* next;
	};

	struct t_Top
	{
		Node* ptr_node = nullptr;
		LONG64 counter = 0;
	};

public:
	~TC_LFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(T data);
	bool Pop(T* result);
	DWORD GetSize() { return mSize; }
	int GetPoolSize() { return mMemoryPool.GetSize(); }
	int GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
	DWORD GetMallocCount() { return mMemoryPool.GetMallocCount(); }
	void linkCheck(int size);
	void cLog(int loginId, t_Top snap_top);

private:
#ifdef VER_CASH_LINE
	alignas(64) t_Top	mTop;
	alignas(64) int		mSize = 0;
#else
	t_Top	mTop;
	int		mSize = 0;
#endif // VER_CASH_LINE	
	procademy::TC_LFObjectPool<Node> mMemoryPool;

	
};

template <typename T>
inline TC_LFStack<T>::~TC_LFStack()
{
	Node* node = mTop.ptr_node;
	int count = 0;

	while (node != nullptr)
	{
		Node* cur = node;

		node = node->next;

		mMemoryPool.Free(cur);

		count++;
	}

	wprintf_s(L"[Size: %u] [Counter: %d]\n", mSize, count);
}

template <typename T>
void TC_LFStack<T>::Push(T data)
{
	// prerequisite
	alignas(16) t_Top top;
	top.counter = -9999;
	Node* ptop;

	//cLog(PUSH_DEBUG, top);
	Node* node = mMemoryPool.Alloc();
	//cLog(PUSH_DEBUG + 10, top);
	node->data = data;

	do
	{
		ptop = mTop.ptr_node;		// Node Address -> Low Part
		top.ptr_node = ptop;
		node->next = ptop;
		//cLog(PUSH_DEBUG + 20, top);
	} while (InterlockedCompareExchangePointer((PVOID*)&mTop.ptr_node, node, ptop) != ptop);

	//cLog(PUSH_DEBUG + 30, top);
	InterlockedIncrement((long*)&mSize);
	//_Log(PUSH_DEBUG + 40, GetCurrentThreadId(), mSize, mTop.counter, mTop.ptr_node, top, node);
}

template <typename T>
bool TC_LFStack<T>::Pop(T* result)
{
	alignas(16) t_Top top;
	Node* next = nullptr;
	USHORT idx = 0;

	if (mSize <= 0)
	{
		result = nullptr;
		return false;
	}

	//cLog(POP_DEBUG, top);
	InterlockedDecrement((long*)&mSize);
	//cLog(POP_DEBUG + 10, top);

	if (mSize < 0)
	{
		result = nullptr;
		//_Log(POP_DEBUG + 20, GetCurrentThreadId(), mSize, mTop.counter, mTop.ptr_node, nullptr);
		InterlockedIncrement((long*)&mSize);
		//_Log(POP_DEBUG + 30, GetCurrentThreadId(), mSize, mTop.counter, mTop.ptr_node, nullptr);
		return false;
	}

	do
	{
		top.counter = mTop.counter;
		top.ptr_node = mTop.ptr_node;
		//cLog(POP_DEBUG + 30, top);
		//_Log(POP_DEBUG + 20, GetCurrentThreadId(), mSize, mTop.counter, -9999, mTop.ptr_node, nullptr, nullptr);
		//top.ptr_node = mTop.ptr_node;		// Node Address -> Low Part
		//cLog(POP_DEBUG + 20, top);
		//top.counter = mTop.counter;				// Counter -> High Part
		//cLog(POP_DEBUG + 30, top);
		//if (top.ptr_node == nullptr)
		//{
		//	idx = finder_log();
		//	CRASH();
		//}
		next = top.ptr_node->next;
		//cLog(POP_DEBUG + 40, top);

	} while (InterlockedCompareExchange128((LONG64*)&mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

	//cLog(POP_DEBUG + 50, top);
	*result = top.ptr_node->data;
	mMemoryPool.Free(top.ptr_node);

	return true;
}

template<typename T>
inline void TC_LFStack<T>::linkCheck(int size)
{
	Node* node = mTop.ptr_node;

	int count = 0;

	while (node != nullptr)
	{
		count++;
		node = node->next;
	}

	if (count == size)
	{
		wprintf_s(L"Pushed Successfully\n=====================\n");
	}
}

template<typename T>
inline void TC_LFStack<T>::cLog(int loginId, t_Top snap_top)
{
	t_Top curmTop;
	curmTop.ptr_node = mTop.ptr_node;
	curmTop.counter = mTop.counter;

	if (curmTop.ptr_node == nullptr)
	{
		if (snap_top.ptr_node == nullptr)
		{
			_Log(loginId, GetCurrentThreadId(), mSize, curmTop.counter, snap_top.counter, curmTop.ptr_node, nullptr, snap_top.ptr_node, nullptr);
		}
		else
		{
			_Log(loginId, GetCurrentThreadId(), mSize, curmTop.counter, snap_top.counter, curmTop.ptr_node, nullptr, snap_top.ptr_node, snap_top.ptr_node->next);
		}
	}
	else
	{
		if (snap_top.ptr_node == nullptr)
		{
			_Log(loginId, GetCurrentThreadId(), mSize, curmTop.counter, snap_top.counter, curmTop.ptr_node, curmTop.ptr_node->next, snap_top.ptr_node, nullptr);
		}
		else
		{
			_Log(loginId, GetCurrentThreadId(), mSize, curmTop.counter, snap_top.counter, curmTop.ptr_node, curmTop.ptr_node->next, snap_top.ptr_node, snap_top.ptr_node->next);
		}
	}
}

