#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

#define VIRTUAL_ADDRESS (0x7FFFFFFFFFF)
#define COUNTER_BIT (0xFFFFF80000000000)
#define COUNTER_BIT_INCREMENT (0x80000000000)
#define COUNTER_BEGIN_BIT (43)

#define dfGetNodeAddress(address) ((Node*)(VIRTUAL_ADDRESS & address))

namespace procademy
{
	template <typename DATA>
	class TC_LFQueue64
	{
	private:
		struct Node
		{
			DATA data;
			Node* next;
		};

		struct t_Top
		{
			Node* ptr_node = nullptr;
			LONG64 counter = -9999;
		};

		alignas(64) LONG64	mHead;        // 시작노드를 포인트한다.
		alignas(64) LONG64	mTail;        // 마지막노드를 포인트한다.
		alignas(64) int		mSize = 0;
		alignas(64) procademy::TC_LFObjectPool<Node> mMemoryPool;

	public:
		enum {
			LOGIC_DEQUEUE = 10000,
			LOGIC_ENQUEUE = 20000
		};

		TC_LFQueue64();
		~TC_LFQueue64();
		void		Enqueue(DATA data);
		bool		Dequeue(DATA* data);
		DWORD		Peek(DATA arr[], DWORD size);
		bool		IsEmpty() { return mSize == 0; }
		DWORD		GetSize() { return mSize; }
		DWORD		GetPoolCapacity() { return mMemoryPool.GetCapacity(); }
		DWORD		GetPoolSize() { return mMemoryPool.GetSize(); }
		void		linkCheck(int size);

	private:
		Node*		GetNodeAddress(LONG64 address);
		LONG64		GetNodeCounter(LONG64 address);
	};

	template<typename DATA>
	inline TC_LFQueue64<DATA>::TC_LFQueue64()
	{
		Node* dummy = mMemoryPool.Alloc();

		dummy->next = nullptr;
		mHead = (LONG64)dummy;
		mTail = (LONG64)dummy;
	}

	template<typename DATA>
	inline TC_LFQueue64<DATA>::~TC_LFQueue64()
	{
		//Node* node = GetNodeAddress(mHead);
		Node* node = GetNodeAddress(mHead);
		int count = 0;
		while (node != nullptr)
		{
			Node* temp = node;

			node = node->next;
			count++;
			mMemoryPool.Free(temp);
		}
	}

	template<typename DATA>
	inline void TC_LFQueue64<DATA>::Enqueue(DATA data)
	{
		LONG64		tail;
		LONG64		nextTail;
		Node*		pNode = mMemoryPool.Alloc();
		Node*		pNext;
		Node*		pTail;

		pNode->data = data;
		pNode->next = nullptr;

		//Log(LOGIC_ENQUEUE, top, node);

		while (1)
		{
			tail = mTail;
			pTail = GetNodeAddress(tail);
			pNext = pTail->next;

			if (pNext == nullptr)
			{
				if (InterlockedCompareExchangePointer((PVOID*)&pTail->next, pNode, nullptr) == nullptr)
				{
					InterlockedIncrement((long*)&mSize);
					//Log(LOGIC_ENQUEUE + 50, top, next);

					nextTail = (GetNodeCounter(tail) + COUNTER_BIT_INCREMENT) | (LONG64)pNode;
					InterlockedCompareExchange64(&mTail, nextTail, tail);

					break;
				}
			}
			else
			{
				nextTail = (GetNodeCounter(tail) + COUNTER_BIT_INCREMENT) | (LONG64)pNext;
				InterlockedCompareExchange64(&mTail, nextTail, tail);
			}
		}
	}

	template<typename DATA>
	inline bool TC_LFQueue64<DATA>::Dequeue(DATA* data)
	{
		LONG64 head;
		LONG64 tail;
		LONG64 nextTail;
		LONG64 nextHead;
		Node* pNext;
		Node* pTail;
		Node* pHead;

		if (InterlockedDecrement((long*)&mSize) < 0)
		{
			InterlockedIncrement((long*)&mSize);

			return false;
		}

		while (1)
		{
			head = mHead;
			pHead = GetNodeAddress(head);

			pNext = pHead->next;

			tail = mTail;
			pTail = GetNodeAddress(tail);

			if (pHead == pTail || pNext == nullptr)
			{
				//MoveTail();
				pNext = pTail->next;

				if (pNext != nullptr)
				{
					nextTail = (GetNodeCounter(tail) + COUNTER_BIT_INCREMENT) | (LONG64)pNext;
					InterlockedCompareExchange64(&mTail, nextTail, tail);
				}
			}
			else
			{
				*data = pNext->data;

				nextHead = ((head & COUNTER_BIT) + COUNTER_BIT_INCREMENT) | (LONG64)(pHead->next);

				if (InterlockedCompareExchange64(&mHead, nextHead, head) == head)
				{
					break;
				}
			}
		}

		mMemoryPool.Free(pHead);

		return true;
	}

	template<typename DATA>
	inline DWORD TC_LFQueue64<DATA>::Peek(DATA arr[], DWORD size)
	{
		DWORD i;
		Node* pHead = GetNodeAddress(mHead)->next;

		for (i = 0; i < size; ++i)
		{
			if (pHead == nullptr)
			{
				return i;
			}

			arr[i] = pHead->data;
			pHead = pHead->next;
		}

		return i;
	}

	template<typename DATA>
	inline void TC_LFQueue64<DATA>::linkCheck(int size)
	{
		Node* node = GetNodeAddress(mHead)->next;

		int count = 0;

		while (node != nullptr)
		{
			count++;
			node =node->next;
		}

		if (count == size)
		{
			wprintf_s(L"Enqueued Successfully\n=====================\n");
		}
	}
	template<typename DATA>
	inline typename TC_LFQueue64<DATA>::Node* TC_LFQueue64<DATA>::GetNodeAddress(LONG64 address)
	{
		return (Node*)(VIRTUAL_ADDRESS & address);
	}
	template<typename DATA>
	inline typename LONG64 TC_LFQueue64<DATA>::GetNodeCounter(LONG64 address)
	{
		return (COUNTER_BIT & address);
	}
}
