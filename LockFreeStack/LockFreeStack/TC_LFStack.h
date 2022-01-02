#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

//struct stackDebug
//{
//	int logicId;
//	int threadId;
//	void* mTop;
//	void* mTopNext;
//	void* snapTop;
//	void* snapTopNext;
//	void* enqueueNode;
//	void* dequeueNode;
//	int size;
//	int counter;
//	int snapCounter;
//};
//
//stackDebug g_debug[USHRT_MAX + 1];
//USHORT g_index;
//
//void _debug(
//	int logicId,
//	int threadId,
//	void* mTop,
//	void* mTopNext,
//	void* snapTop,
//	void* snapTopNext,
//	void* enqueueNode,
//	void* dequeueNode,
//	int size,
//	int counter,
//	int snapCounter
//)
//{
//	USHORT index = InterlockedIncrement16((short*)&g_index);
//
//	g_debug[index].logicId = logicId;
//	g_debug[index].threadId = threadId;
//	g_debug[index].mTop = mTop;
//	g_debug[index].mTopNext = mTopNext;
//	g_debug[index].snapTop = snapTop;
//	g_debug[index].snapTopNext = snapTopNext;
//	g_debug[index].enqueueNode = enqueueNode;
//	g_debug[index].dequeueNode = dequeueNode;
//	g_debug[index].size = size;
//	g_debug[index].counter = counter;
//	g_debug[index].snapCounter = snapCounter;
//}

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
	void linkCheck(int size);

private:
	alignas(64) t_Top	mTop;
	alignas(64) int		mSize = 0;
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
}

template <typename T>
void TC_LFStack<T>::Push(T data)
{
	// prerequisite
	Node* ptop;

	Node* node = mMemoryPool.Alloc();
	node->data = data;

	do
	{
		ptop = mTop.ptr_node;		// Node Address -> Low Part
		node->next = ptop;

		//_debug(PUSH_DEBUG, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, ptop, ptop == nullptr? nullptr : ptop->next, node, nullptr, mSize, mTop.counter, 0);
	} while (InterlockedCompareExchangePointer((PVOID*)&mTop.ptr_node, node, ptop) != ptop);
	//_debug(PUSH_DEBUG + 10, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, ptop, ptop == nullptr ? nullptr : ptop->next, node, nullptr, mSize, mTop.counter, 0);

	InterlockedIncrement((long*)&mSize);
}

template <typename T>
bool TC_LFStack<T>::Pop(T* result)
{
	alignas(16) t_Top top;
	Node* next = nullptr;
	USHORT idx = 0;

	if (InterlockedDecrement((long*)&mSize) < 0)
	{
		result = nullptr;
		InterlockedIncrement((long*)&mSize);
		return false;
	}

	do
	{
		top.counter = mTop.counter;
		top.ptr_node = mTop.ptr_node;
		//top.counter = mTop.counter;
		next = top.ptr_node->next;

		//_debug(POP_DEBUG, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, top.ptr_node, next, nullptr, top.ptr_node, mSize, mTop.counter, top.counter);
	} while (InterlockedCompareExchange128((LONG64*)&mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);
	//_debug(POP_DEBUG + 10, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, top.ptr_node, next, nullptr, top.ptr_node, mSize, mTop.counter, top.counter);
	*result = top.ptr_node->data;

	mMemoryPool.Free(top.ptr_node);

	//Node* ptop;

	//do
	//{
	//	ptop = mTop.ptr_node;		// Node Address -> Low Part
	//	next = ptop->next;

	//	_debug(POP_DEBUG, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, ptop, next, nullptr, ptop, mSize, 0, 0);

	//	if (ptop == mTop.ptr_node && next != mTop.ptr_node->next)
	//	{
	//		_debug(POP_DEBUG + 10, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr ? nullptr : mTop.ptr_node->next, ptop, next, nullptr, ptop, mSize, 0, 0);
	//	}

	//} while (InterlockedCompareExchangePointer((PVOID*)&mTop.ptr_node, next, ptop) != ptop);

	//_debug(POP_DEBUG + 20, GetCurrentThreadId(), mTop.ptr_node, mTop.ptr_node == nullptr? nullptr : mTop.ptr_node->next, ptop, next, nullptr, ptop, mSize, 0, 0);

	//*result = ptop->data;

	//mMemoryPool.Free(ptop);

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
