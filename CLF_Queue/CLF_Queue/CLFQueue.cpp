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
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);
	debug[*index].type = 'E';

	InterlockedIncrement((DWORD*)&mSize);
	debug[*index].size1 = mSize;

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
		
		debug[*index].address1 = tail;

		if (InterlockedCompareExchangePointer((PVOID*)&tail->next, node, nullptr) == nullptr)
		{
			debug[*index].address2 = mTail;
			if (InterlockedCompareExchangePointer((PVOID*)&mTail, node, tail) != tail)
			{
				CRASH();
			}
			debug[*index].address3 = mTail;
			break;
		}
	}

	(*index)++;
}

bool CLFQueue::Dequeue(ULONG64* data)
{
	st_DEBUG* debug = (st_DEBUG*)TlsGetValue(g_records);
	USHORT* index = (USHORT*)TlsGetValue(g_index);

	debug[*index].type = 'D';

	InterlockedDecrement((DWORD*)&mSize);
	debug[*index].size1 = mSize;

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
		debug[*index].address1 = top;
	} while (InterlockedCompareExchangePointer((PVOID*)&mHead, next, top) != top);
	debug[*index].address2 = mHead;

	*data = next->data;
	mMemoryPool.Free(top);

	(*index)++;

	return true;
}
