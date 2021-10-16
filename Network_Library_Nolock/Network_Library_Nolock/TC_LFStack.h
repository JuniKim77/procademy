#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

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

	Node* node = mMemoryPool.Alloc();
	node->data = data;

	do
	{
		ptop = mTop.ptr_node;		// Node Address -> Low Part
		top.ptr_node = ptop;
		node->next = ptop;
	} while (InterlockedCompareExchangePointer((PVOID*)&mTop.ptr_node, node, ptop) != ptop);

	InterlockedIncrement((long*)&mSize);
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

	InterlockedDecrement((long*)&mSize);

	if (mSize < 0)
	{
		result = nullptr;
		InterlockedIncrement((long*)&mSize);
		return false;
	}

	do
	{
		top.counter = mTop.counter;
		top.ptr_node = mTop.ptr_node;
		next = top.ptr_node->next;
	} while (InterlockedCompareExchange128((LONG64*)&mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

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
