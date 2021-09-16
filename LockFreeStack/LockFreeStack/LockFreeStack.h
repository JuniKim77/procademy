#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"
//template <typename T>
//class CLFStack
//{
//public:
//	bool IsEmpty() { return mSize == 0; }
//	void Push(T data);
//	T Pop();
//	DWORD GetSize() { return mSize; }
//
//public:
//	static T sDummy;
//
//private:
//
//private:
//	struct tNode;
//	struct Node
//	{
//		T data;
//		tNode* next;
//	};
//	struct tNode
//	{
//		UINT64 counter;
//		Node* node;
//	};
//	tNode* mTop = nullptr;
//
//	DWORD mSize = 0;
//};
//
//template<typename T>
//T CLFStack<T>::sDummy = T();
//
//template<typename T>
//inline void CLFStack<T>::Push(T data)
//{
//	// prerequisite
//	Node* node = new Node;
//	node->data = data;
//	tNode* tnode = new tNode;
//	tnode->node = node;
//
//	tNode* t;
//	do
//	{
//		t = mTop;
//		node->next = t;
//		tnode->counter = t->counter + 1;
//	} while (InterlockedCompareExchange128(mTop, &tnode->counter, tnode->node, t) == 0);
//
//	InterlockedIncrement(&mSize);
//}
//
//template<typename T>
//inline T CLFStack<T>::Pop()
//{
//	tNode* tnode;
//	tNode* next;
//	T ret;
//
//	if (IsEmpty())
//	{
//		return sDummy;
//	}
//
//	do
//	{
//		tnode = mTop;
//		ret = tnode->node->data;
//		next = tnode->node->next;
//	} while (InterlockedCompareExchange128(mTop, &next->counter, next->node, tnode) == 0);
//	
//	InterlockedDecrement(&mSize);
//
//	delete tnode;
//
//	return ret;
//}

class CLFStack
{
public:
	~CLFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(ULONG64 data);
	bool Pop(ULONG64* result);
	DWORD GetSize() { return mSize; }

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
	void* top;
	Node* node = mMemoryPool.Alloc();
	node->data = data;	

	do
	{
		top = mTop.ptr_node;		// Node Address -> Low Part
		node->next = (Node*)top;
	} while (InterlockedCompareExchange64((LONG64*)&mTop, (LONG64)node, (LONG64)top) != (LONG64)top);

	InterlockedIncrement((DWORD*)&mSize);
}

bool CLFStack::Pop(ULONG64* result)
{
	alignas(16) t_Top top;
	Node* next;

	if (InterlockedDecrement((DWORD*)&mSize) < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);
		return false;
	}

	do
	{
		top.ptr_node = mTop.ptr_node;		// Node Address -> Low Part
		top.counter = mTop.counter;				// Counter -> High Part

		*result = ((Node*)top.ptr_node)->data;
		next = top.ptr_node->next;

	} while (InterlockedCompareExchange128((LONG64*)&mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

	mMemoryPool.Free(top.ptr_node);

	return true;
}
