#pragma once

#include <wtypes.h>

template <typename T>
class CLFStack
{
public:
	bool IsEmpty();
	void Push(T data);
	T Pop();
	DWORD GetSize() { return mSize; }

public:
	static T sDummy;

private:

private:
	struct Node
	{
		T data;
		Node* next;
	};
	Node* mTop = nullptr;

	DWORD mSize = 0;
};

template<typename T>
T CLFStack<T>::sDummy = T();

template<typename T>
inline bool CLFStack<T>::IsEmpty()
{
	return mSize == 0;
}

template<typename T>
inline void CLFStack<T>::Push(T data)
{
	// prerequisite
	Node* node = new Node;
	node->data = data;
	Node* t = mTop;
	node->next = t;

	mTop = node;
	mSize++;
}

template<typename T>
inline T CLFStack<T>::Pop()
{
	if (IsEmpty())
	{
		return sDummy;
	}

	Node* node = mTop;
	T ret = node->data;
	mTop = mTop->next;
	mSize--;

	delete node;

	return ret;
}
