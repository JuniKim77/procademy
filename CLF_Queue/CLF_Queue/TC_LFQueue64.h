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

	private:
		Node*		GetNodeAddress(Node* address);
		INT64		GetNodeCounter(Node* address);
		Node*		SetNodeCounter(Node* address, INT64 counter);
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
		Node* node = GetNodeAddress(mHead);
		int count = 0;
		while (node != nullptr)
		{
			Node* temp = GetNodeAddress(node);

			node = GetNodeAddress(node)->next;
			count++;
			mMemoryPool.Free(temp);
		}
	}

	template<typename DATA>
	inline void TC_LFQueue64<DATA>::Enqueue(DATA data)
	{
		Node* tail;
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
			tail = mTail;
			next = GetNodeAddress(tail)->next;

			if (next == nullptr)
			{
				//InterlockedCompareExchangePointer((PVOID*)&top.ptr_node->next, node, nullptr) == nullptr)
				if (InterlockedCompareExchange64((LONG64*)&GetNodeAddress(tail)->next, (LONG64)node, NULL) == NULL)
				{
					InterlockedIncrement((long*)&mSize);
					//Log(LOGIC_ENQUEUE + 50, top, next);

					/*if (next != nullptr)
					{
						InterlockedCompareExchange128((LONG64*)&mTail, top.counter + 1, (LONG64)next, (LONG64*)&top);
					}*/
					next = SetNodeCounter(tail, GetNodeCounter(tail) + 1);

					InterlockedCompareExchange64((LONG64*)&mTail, (LONG64)next, (LONG64)tail);

					break;
				}
			}
			else
			{
				/*if (next != nullptr)
				{
					InterlockedCompareExchange128((LONG64*)&mTail, top.counter + 1, (LONG64)next, (LONG64*)&top);
				}*/

				next = SetNodeCounter(tail, GetNodeCounter(tail) + 1);

				InterlockedCompareExchange64((LONG64*)&mTail, (LONG64)next, (LONG64)tail);
			}
		}
	}

	template<typename DATA>
	inline bool TC_LFQueue64<DATA>::Dequeue(DATA* data)
	{
		Node* top;
		Node* tail;
		INT64 temp;
		Node* next;

		if (InterlockedDecrement((long*)&mSize) < 0)
		{
			InterlockedIncrement((long*)&mSize);

			return false;
		}

		while (1)
		{
			/*top.ptr_node = mHead.ptr_node;
			next = top.ptr_node->next;

			tail.counter = mTail.counter;
			tail.ptr_node = mTail.ptr_node;*/

			top = mHead;
			next = GetNodeAddress(top)->next;

			tail = mTail;

			if (top == tail || next == nullptr)
			{
				/*if (tailNext != nullptr)
				{
					InterlockedCompareExchange128((LONG64*)&mTail, tail.counter + 1, (LONG64)tailNext, (LONG64*)&tail);
				}*/
				next = SetNodeCounter(next, GetNodeCounter(top) + 1);

				InterlockedCompareExchange64((LONG64*)&mTail, (LONG64)next, (LONG64)tail);
			}
			else
			{
				*data = GetNodeAddress(next)->data;
				//if (InterlockedCompareExchange128((LONG64*)&mHead, top.counter + 1, (LONG64)next, (LONG64*)&top))
				//{
				//	//Log(LOGIC_DEQUEUE + 50, top, next, true);
				//	break;
				//}
				next = SetNodeCounter(next, GetNodeCounter(top) + 1);

				if (InterlockedCompareExchange64((LONG64*)&mHead, (LONG64)next, (LONG64)top) == (LONG64)top)
				{
					break;
				}
			}
		}

		mMemoryPool.Free(GetNodeAddress(top));

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
		Node* node = GetNodeAddress(mHead)->next;

		int count = 0;

		while (node != nullptr)
		{
			count++;
			node = GetNodeAddress(node)->next;
		}

		if (count == size)
		{
			wprintf_s(L"Enqueued Successfully\n=====================\n");
		}
	}
	template<typename DATA>
	inline typename TC_LFQueue64<DATA>::Node* TC_LFQueue64<DATA>::GetNodeAddress(TC_LFQueue64<DATA>::Node* address)
	{
		return (Node*)(VIRTUAL_ADDRESS & (INT64)address);
	}
	template<typename DATA>
	inline typename INT64 TC_LFQueue64<DATA>::GetNodeCounter(TC_LFQueue64<DATA>::Node* address)
	{
		return ((COUNTER_BIT & (INT64)address) >> COUNTER_BEGIN_BIT);
	}
	template<typename DATA>
	inline typename TC_LFQueue64<DATA>::Node* TC_LFQueue64<DATA>::SetNodeCounter(TC_LFQueue64<DATA>::Node* address, INT64 counter)
	{
		counter <<= COUNTER_BEGIN_BIT;

		return (Node*)((INT64)address | counter);
	}
}
