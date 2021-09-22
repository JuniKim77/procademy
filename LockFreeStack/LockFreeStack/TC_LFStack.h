#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

extern DWORD g_records;
extern DWORD g_index;

template <typename T>
class TC_LFStack
{
public:
	~TC_LFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(T data);
	bool Pop(T* result, st_DEBUG** info);
	DWORD GetSize() { return mSize; }
	int GetPoolSize() { return mMemoryPool.GetSize(); }
	int GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
	DWORD GetMallocCount() { return mMemoryPool.GetMallocCount(); }
	void linkCheck(int size);

private:

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

	alignas(16) t_Top mTop;
	procademy::TC_LFObjectPool<Node> mMemoryPool;

	int mSize = 0;
};

template <typename T>
inline TC_LFStack<T>::~TC_LFStack()
{
	Node* node = mTop.ptr_node;
	wprintf_s(L"[Size: %u] [Counter: %lld]\n", mSize, mTop.counter);
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
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);

	// prerequisite
	Node* top;
	Node* node = mMemoryPool.Alloc();
	node->data = data;

	do
	{
		top = mTop.ptr_node;		// Node Address -> Low Part
		node->next = top;
		debug[*index].size1 = mSize;
		debug[*index].address1 = top;
		debug[*index].counter1 = mTop.counter;
	} while (InterlockedCompareExchangePointer((PVOID*)&mTop.ptr_node, node, top) != top);
	debug[*index].size2 = mSize;
	InterlockedIncrement((long*)&mSize);
	debug[*index].size3 = mSize;

	(*index)++;
}

template <typename T>
bool TC_LFStack<T>::Pop(T* result, st_DEBUG** info)
{
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);

	alignas(16) t_Top top;
	Node* next = nullptr;

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
		do
		{
			top.ptr_node = mTop.ptr_node;		// Node Address -> Low Part
			top.counter = mTop.counter;				// Counter -> High Part
		} while (top.ptr_node != mTop.ptr_node);
		next = top.ptr_node->next;
		debug[*index].size1 = mSize;
		debug[*index].address1 = top.ptr_node;
		debug[*index].counter1 = mTop.counter;
	} while (InterlockedCompareExchange128((LONG64*)&mTop, mTop.counter + 1, (LONG64)next, (LONG64*)&top) == 0);
	debug[*index].size2 = mSize;

	*result = top.ptr_node->data;
	mMemoryPool.Free(top.ptr_node);
	debug[*index].size3 = -100;

	*info = &debug[*index];

	(*index)++;

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
