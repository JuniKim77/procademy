#pragma once

#include <wtypes.h>

template <typename T>
class CLFStack
{
public:
	~CLFStack();
	bool IsEmpty() { return mSize == 0; }
	void Push(T data);
	T Pop();
	DWORD GetSize() { return mSize; }

public:
	static T sDummy;

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

template<typename T>
T CLFStack<T>::sDummy = T();

template<typename T>
inline CLFStack<T>::~CLFStack()
{
	Node* node = (Node*)mTop[0];
	wprintf_s(L"[Size: %u] [Counter: %lld]\n", mSize, mTop[1]);
	int count = 0;
	while (node != nullptr)
	{
		Node* cur = node;

		node = node->next;

		delete cur;
		++count;
	}
	wprintf_s(L"=======\n[Size: %u] [Counter: %lld]\n", count, mTop[1]);
}

template<typename T>
inline void CLFStack<T>::Push(T data)
{
	// prerequisite
	Node* node = new Node;
	node->data = data;
	LONG64 top[2];

	do
	{
		top[0] = mTop[0];		// Node Address -> Low Part
		top[1] = mTop[1];		// Counter -> High Part
		node->next = (Node*)top[0];
	} while (InterlockedCompareExchange128(mTop, top[1] + 1, (LONG64)node, top) == 0);

	InterlockedIncrement(&mSize);
}

template<typename T>
inline T CLFStack<T>::Pop()
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
	// 메모리에다가 하나하나 기록할 것.
	// 확인할 수 있는 모든 걸 다 기록할 것
	// new delete로는 해결이 안됨. 힙을 대상으로 동기화가 들어감...

	return ret;
}
