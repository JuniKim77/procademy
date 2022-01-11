#pragma once
#include <stdio.h>
#include <wtypes.h>
#include "TC_LFObjectPool.h"

//struct queueDebug
//{
//	int logicId;
//	int threadId;
//	void* mHead;
//	void* mHeadNext;
//	void* mTail;
//	void* mTailNext;
//	void* snapNode;
//	void* snapNext;
//	void* enqueueNode;
//	void* dequeueNode;
//	LONG64 counter;
//	LONG64 snapCounter;
//	DWORD mSize;
//};
//
//queueDebug g_queueLog[USHRT_MAX + 1];
//USHORT g_idx;
//
//void _QueueLog(
//	int logicId,
//	int threadId,
//	void* mHead,
//	void* mHeadNext,
//	void* mTail,
//	void* mTailNext,
//	void* snapNode,
//	void* snapNext,
//	void* enqueueNode,
//	void* dequeueNode,
//	LONG64 counter,
//	LONG64 snapCounter,
//	DWORD mSize
//)
//{
//	USHORT index = InterlockedIncrement16((short*)&g_idx);
//
//	g_queueLog[index].logicId = logicId;
//	g_queueLog[index].threadId = threadId;
//	g_queueLog[index].mHead = mHead;
//	g_queueLog[index].mHeadNext = mHeadNext;
//	g_queueLog[index].mTail = mTail;
//	g_queueLog[index].mTailNext = mTailNext;
//	g_queueLog[index].snapNode = snapNode;
//	g_queueLog[index].snapNext = snapNext;
//	g_queueLog[index].enqueueNode = enqueueNode;
//	g_queueLog[index].dequeueNode = dequeueNode;
//	g_queueLog[index].counter = counter;
//	g_queueLog[index].snapCounter = snapCounter;
//	g_queueLog[index].mSize = mSize;
//}

namespace procademy
{
	template <typename DATA>
	class TC_LFQueue
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

		t_Top	mHead;        // 시작노드를 포인트한다. // Interlock
		alignas(64) t_Top	mTail;        // 마지막노드를 포인트한다.
		alignas(64) int		mSize = 0;

	public:
		alignas(64) procademy::TC_LFObjectPool<Node> mMemoryPool;
		enum {
			LOGIC_DEQUEUE = 10000,
			LOGIC_ENQUEUE = 20000
		};

		TC_LFQueue();
		~TC_LFQueue();
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
	inline TC_LFQueue<DATA>::TC_LFQueue()
	{
		Node* dummy = mMemoryPool.Alloc();

		dummy->next = nullptr;
		mHead.ptr_node = dummy;
		mHead.counter = 0;
		mTail.ptr_node = dummy;
		mTail.counter = 0;
	}

	template<typename DATA>
	inline TC_LFQueue<DATA>::~TC_LFQueue()
	{
		Node* node = mHead.ptr_node;
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
	inline void TC_LFQueue<DATA>::Enqueue(DATA data)
	{
		alignas(16) t_Top tail;
		Node* node = mMemoryPool.Alloc();
		Node* next;

		node->data = data;
		node->next = nullptr;

		//Log(LOGIC_ENQUEUE, top, node);

		while (1)
		{
			tail.counter = mTail.counter;
			tail.ptr_node = mTail.ptr_node;
			next = tail.ptr_node->next;

			if (next == nullptr)
			{
				/*_QueueLog(LOGIC_ENQUEUE, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
					tail.ptr_node, next, node, nullptr, mTail.counter, tail.counter, mSize);*/

				if (InterlockedCompareExchangePointer((PVOID*)&tail.ptr_node->next, node, nullptr) == nullptr)
				{
					InterlockedIncrement((long*)&mSize);
					/*_QueueLog(LOGIC_ENQUEUE + 10, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
						tail.ptr_node, tail.ptr_node->next, node, nullptr, mTail.counter, tail.counter, mSize);*/

					InterlockedCompareExchange128((LONG64*)&mTail, tail.counter + 1, (LONG64)node, (LONG64*)&tail);

					/*_QueueLog(LOGIC_ENQUEUE + 20, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
						tail.ptr_node, node, node, nullptr, mTail.counter, tail.counter, mSize);*/

					break;
				}
			}
			else
			{
				/*_QueueLog(LOGIC_ENQUEUE + 30, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
					tail.ptr_node, next, node, nullptr, mTail.counter, tail.counter, mSize);*/

				InterlockedCompareExchange128((LONG64*)&mTail, tail.counter + 1, (LONG64)next, (LONG64*)&tail);

				/*_QueueLog(LOGIC_ENQUEUE + 40, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
					tail.ptr_node, next, node, nullptr, mTail.counter, tail.counter, mSize);*/
			}
		}
	}

	template<typename DATA>
	inline bool TC_LFQueue<DATA>::Dequeue(DATA* data)
	{
		alignas(16) t_Top head;
		alignas(16) t_Top tail;

		if (InterlockedDecrement((long*)&mSize) < 0)
		{
			InterlockedIncrement((long*)&mSize);

			return false;
		}

		Node* next;
		Node* tailNext;

		while (1)
		{
			head.counter = mHead.counter;
			head.ptr_node = mHead.ptr_node;
			next = head.ptr_node->next;

			tail.counter = mTail.counter;
			tail.ptr_node = mTail.ptr_node;

			if (head.ptr_node == tail.ptr_node || next == nullptr)
			{
				tailNext = tail.ptr_node->next;

				if (tailNext != nullptr)
				{
					/*_QueueLog(LOGIC_DEQUEUE, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
						tail.ptr_node, tailNext, nullptr, next, mTail.counter, tail.counter, mSize);*/

					InterlockedCompareExchange128((LONG64*)&mTail, tail.counter + 1, (LONG64)tailNext, (LONG64*)&tail);

					/*_QueueLog(LOGIC_DEQUEUE + 10, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
						tail.ptr_node, tailNext, nullptr, next, mTail.counter, tail.counter, mSize);*/
				}
			}
			else
			{
				/*_QueueLog(LOGIC_DEQUEUE + 20, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
					head.ptr_node, next, nullptr, head.ptr_node, mHead.counter, head.counter, mSize);*/

				*data = next->data;

				if (InterlockedCompareExchange128((LONG64*)&mHead, head.counter + 1, (LONG64)next, (LONG64*)&head))
				{
					/*_QueueLog(LOGIC_DEQUEUE + 30, GetCurrentThreadId(), mHead.ptr_node, mHead.ptr_node->next, mTail.ptr_node, mTail.ptr_node->next,
						head.ptr_node, next, nullptr, head.ptr_node, mHead.counter, head.counter, mSize);*/

					break;
				}
			}
		}

		mMemoryPool.Free(head.ptr_node);

		return true;
	}

	template<typename DATA>
	inline DWORD TC_LFQueue<DATA>::Peek(DATA arr[], DWORD size)
	{
		DWORD i;
		Node* pHead = mHead.ptr_node->next;
		DWORD curSize = (DWORD)mSize;
		size = size > curSize ? curSize : size;

		for (i = 0; i < size; ++i)
		{
			arr[i] = pHead->data;
			pHead = pHead->next;
		}

		return i;
	}

	template<typename DATA>
	inline void TC_LFQueue<DATA>::linkCheck(int size)
	{
		Node* node = mHead.ptr_node->next;

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
