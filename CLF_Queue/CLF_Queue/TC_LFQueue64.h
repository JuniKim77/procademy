#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

#define VIRTUAL_ADDRESS (0x7FFFFFFFFFF)
#define COUNTER_BIT (0xFFFFF80000000000)
#define COUNTER_BEGIN_BIT (43)

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

		alignas(64) Node*	mHead;        // 시작노드를 포인트한다.
		alignas(64) Node*	mTail;        // 마지막노드를 포인트한다.
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
	};

	template<typename DATA>
	inline TC_LFQueue64<DATA>::TC_LFQueue64()
	{
		Node* dummy = mMemoryPool.Alloc();

		dummy->next = nullptr;
		mHead = dummy;
		mTail = dummy;
	}

	template<typename DATA>
	inline TC_LFQueue64<DATA>::~TC_LFQueue64()
	{
		Node* node = (Node*)(VIRTUAL_ADDRESS & (INT64)mHead);
		int count = 0;
		while (node != nullptr)
		{
			Node* temp = (Node*)(VIRTUAL_ADDRESS & (INT64)node);

			node = (Node*)(VIRTUAL_ADDRESS & (INT64)(node->next));
			count++;
			mMemoryPool.Free(temp);
		}
	}

	template<typename DATA>
	inline void TC_LFQueue64<DATA>::Enqueue(DATA data)
	{
		Node* top;
		Node* node = mMemoryPool.Alloc();
		Node* next;
		INT64 nextCounter;

		node->data = data;
		node->next = nullptr;

		//Log(LOGIC_ENQUEUE, top, node);

		while (1)
		{
			//top.counter = mTail.counter;
			//top.ptr_node = mTail.ptr_node;
			//next = top.ptr_node->next;
			top = mTail;
			next = ((Node*)((INT64)top & VIRTUAL_ADDRESS))->next;
			nextCounter = (COUNTER_BIT & (INT64)top >> COUNTER_BEGIN_BIT) + 1;
			node = (Node*)((INT64)node | (nextCounter << COUNTER_BEGIN_BIT));

			if (next == nullptr)
			{
				if (InterlockedCompareExchange64((LONG64*)&((Node*)((INT64)mTail & VIRTUAL_ADDRESS))->next, (LONG64)node, 0) == 0)
				{
					InterlockedIncrement((long*)&mSize);
					//Log(LOGIC_ENQUEUE + 50, top, next);

					/*if (next != nullptr)
					{
						InterlockedCompareExchange128((LONG64*)&mTail, top.counter + 1, (LONG64)next, (LONG64*)&top);
					}*/

					InterlockedCompareExchange64((LONG64*)&mTail, (LONG64)node, (LONG64)top);

					break;
				}
			}
			else
			{
				/*if (next != nullptr)
				{
					InterlockedCompareExchange128((LONG64*)&mTail, top.counter + 1, (LONG64)next, (LONG64*)&top);
				}*/

				InterlockedCompareExchangePointer((PVOID*)&mTail, next, (PVOID*)&top);
			}
		}
	}

	template<typename DATA>
	inline bool TC_LFQueue64<DATA>::Dequeue(DATA* data)
	{
		Node* top;
		Node* tail;
		INT64 temp;

		if (InterlockedDecrement((long*)&mSize) < 0)
		{
			InterlockedIncrement((long*)&mSize);

			return false;
		}

		Node* next;

		while (1)
		{
			/*top.ptr_node = mHead.ptr_node;
			next = top.ptr_node->next;

			tail.counter = mTail.counter;
			tail.ptr_node = mTail.ptr_node;*/

			top = mHead;
			next = top->next;

			tail = mTail;

			if (top == tail || next == nullptr)
			{
				/*if (tailNext != nullptr)
				{
					InterlockedCompareExchange128((LONG64*)&mTail, tail.counter + 1, (LONG64)tailNext, (LONG64*)&tail);
				}*/
				InterlockedCompareExchangePointer((PVOID*)&mTail, next, (PVOID*)&tail);
			}
			else
			{
				*data = next->data;
				//if (InterlockedCompareExchange128((LONG64*)&mHead, top.counter + 1, (LONG64)next, (LONG64*)&top))
				//{
				//	//Log(LOGIC_DEQUEUE + 50, top, next, true);
				//	break;
				//}
				if (InterlockedCompareExchangePointer((PVOID*)&mHead, next, (PVOID*)&top) == top)
				{
					break;
				}
			}
		}

		temp = VIRTUAL_ADDRESS & (INT64)top;

		mMemoryPool.Free((Node*)temp);

		return true;
	}

	template<typename DATA>
	inline DWORD TC_LFQueue64<DATA>::Peek(DATA arr[], DWORD size)
	{
		DWORD i;
		Node* pHead = mHead.ptr_node->next;

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
		Node* node = mHead->next;

		int count = 0;

		while (node != nullptr)
		{
			count++;
			node = node->next;
		}

		if (count == size)
		{
			wprintf_s(L"Enqueued Successfully\n=====================\n");
		}
	}
}
