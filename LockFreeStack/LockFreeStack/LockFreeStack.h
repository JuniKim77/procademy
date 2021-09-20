#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

class CLFStack
{
public:
	~CLFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(ULONG64 data);
	bool Pop(ULONG64& result);
	DWORD GetSize() { return mSize; }
	DWORD GetMallocSize() { return mMemoryPool.GetMallocSize(); }
	int GetPoolSize() { return mMemoryPool.GetSize(); }
	int GetPoolCapacity() { return mMemoryPool.GetCapacity(); }

private:

private:
	struct Node
	{
		ULONG64 data;
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

inline CLFStack::~CLFStack()
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

void CLFStack::Push(ULONG64 data)
{
	// prerequisite
	Node* top;
	Node* node = mMemoryPool.Alloc();
	node->data = data;	

	do
	{
		top = mTop.ptr_node;		// Node Address -> Low Part
		node->next = top;
	} while (InterlockedCompareExchangePointer((PVOID*)&mTop, node, top) != top);

	InterlockedIncrement((DWORD*)&mSize);
}

bool CLFStack::Pop(ULONG64& result)
{
	alignas(16) t_Top top;
	Node* next;

	InterlockedDecrement((DWORD*)&mSize);

	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);
		return false;
	}

	do
	{
		top.ptr_node = mTop.ptr_node;		// Node Address -> Low Part
		top.counter = mTop.counter;				// Counter -> High Part

		result = top.ptr_node->data;
		next = top.ptr_node->next;

	} while (InterlockedCompareExchange128((LONG64*)&mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

	mMemoryPool.Free(top.ptr_node);

	return true;
}
