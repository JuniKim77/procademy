#include "CLFQueue.h"
#include "CCrashDump.h"

CLFQueue::CLFQueue()
{
	Node* dummy = mMemoryPool.Alloc();

	dummy->next = nullptr;
	mHead = dummy;
	mTail = dummy;
}

CLFQueue::~CLFQueue()
{
	Node* node = mHead;

	while (node != nullptr)
	{
		Node* temp = node;

		node = node->next;

		delete temp;
	}
}

void CLFQueue::Enqueue(ULONG64 data)
{
	InterlockedIncrement((DWORD*)&mSize);

	Node* node = mMemoryPool.Alloc();
	Node* tail;
	Node* next;

	node->data = data;
	node->next = nullptr;

	while (1)
	{
		do
		{
			tail = mTail;
			next = tail->next;
		} while (next != nullptr);

		if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, nullptr) == nullptr)
		{
			if (InterlockedCompareExchangePointer((PVOID*)&mTail, node, tail) != tail)
			{
				CRASH();
			}
			break;
		}
	}
}

bool CLFQueue::Dequeue(ULONG64* data)
{
	InterlockedDecrement((DWORD*)&mSize);

	if (mSize < 0)
	{
		InterlockedIncrement((DWORD*)&mSize);

		return false;
	}

	Node* top;
	Node* next;

	do
	{
		top = mHead;
		next = top->next;
	} while (InterlockedCompareExchangePointer((PVOID*)&mHead, next, top) != top);

	*data = next->data;
	mMemoryPool.Free(top);

	return true;
}
