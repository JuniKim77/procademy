#pragma once
#include <stdio.h>
#include <wtypes.h>

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
	CLFStack();
	~CLFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(int data);
	bool Pop(int* result);
	DWORD GetSize() { return mSize; }

private:

private:
	struct Node
	{
		int data;
		Node* next;
	};

	struct t_Top
	{
		Node* ptr_node;
		LONG64 counter;
	};
	
	t_Top* mTop;

	DWORD mSize = 0;
};

inline CLFStack::CLFStack()
{
	mTop = new t_Top;

	mTop->ptr_node = nullptr;
	mTop->counter = 0;
}

inline CLFStack::~CLFStack()
{
	Node* node = mTop->ptr_node;
	wprintf_s(L"[Size: %u] [Counter: %lld]\n", mSize, mTop->counter);
	int count = 0;
	while (node != nullptr)
	{
		Node* cur = node;

		node = node->next;

		delete cur;

		count++;
	}

	wprintf_s(L"[Size: %u] [Counter: %d]\n", mSize, count);

	if (mTop != nullptr)
	{
		delete mTop;
	}
}

void CLFStack::Push(int data)
{
	// prerequisite
	Node* node = new Node;
	node->data = data;
	t_Top top;

	do
	{
		top.ptr_node = mTop->ptr_node;		// Node Address -> Low Part
		top.counter = mTop->counter;		// Counter -> High Part
		node->next = top.ptr_node;
	} while (InterlockedCompareExchange128((LONG64*)(mTop), top.counter + 1, (LONG64)node, (LONG64*)&top) == 0);

	InterlockedIncrement(&mSize);
}

bool CLFStack::Pop(int* result)
{
	t_Top top;
	Node* next;

	if (InterlockedDecrement(&mSize) < 0)
	{
		InterlockedIncrement(&mSize);
		return false;
	}

	do
	{
		top.ptr_node = mTop->ptr_node;		// Node Address -> Low Part
		top.counter = mTop->counter;		// Counter -> High Part
		*result = ((Node*)top.ptr_node)->data;
		next = ((Node*)top.ptr_node)->next;
	} while (InterlockedCompareExchange128((LONG64*)mTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

	delete (Node*)top.ptr_node;

	return true;
}
