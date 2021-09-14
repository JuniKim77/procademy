#pragma once

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
	~CLFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(int data);
	int Pop();
	DWORD GetSize() { return mSize; }

private:

private:
	struct Node
	{
		int data;
		Node* next;
	};
	
	LONG64 mTop[2] = { 0, };

	DWORD mSize = 0;
};

inline CLFStack::~CLFStack()
{
	Node* node = (Node*)mTop[0];
	wprintf_s(L"[Size: %d] [Counter: %d]\n", mSize, mTop[1]);
	while (node != nullptr)
	{
		Node* cur = node;

		node = node->next;

		delete cur;
	}
}

void CLFStack::Push(int data)
{
	// prerequisite
	Node* node = new Node;
	node->data = data;
	LONG64 top[2];
	LONG64 nCount;

	do
	{
		top[0] = mTop[0];		// Node Address -> Low Part
		top[1] = mTop[1];		// Counter -> High Part
		node->next = (Node*)top[0];
	} while (InterlockedCompareExchange128(mTop, top[1] + 1, (LONG64)node, top) == 0);

	InterlockedIncrement(&mSize);
}

int CLFStack::Pop()
{
	LONG64 top[2];
	Node* next;
	int ret;

	if (IsEmpty())
	{
		return 0;
	}

	do
	{
		top[0] = mTop[0];
		top[1] = mTop[1];
		ret = ((Node*)top[0])->data;
		next = ((Node*)top[0])->next;
	} while (InterlockedCompareExchange128(mTop, top[1] + 1, (LONG64)next, top) == 0);

	InterlockedDecrement(&mSize);

	delete (Node*)top[0];

	return ret;
}
