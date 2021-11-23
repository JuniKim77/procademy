#pragma once
#define VER_CASH_LINE

#include <new.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include "CCrashDump.h"

#define CHECKSUM_OVER (0xAAAAAAAA)

namespace procademy
{
	template <typename DATA>
	class TC_LFObjectPool
	{
	private:
		struct st_BLOCK_NODE
		{
			st_BLOCK_NODE()
			{
				stpNextBlock = NULL;
			}

			DATA data;
			void* code;
			st_BLOCK_NODE* stpNextBlock;
			unsigned int checkSum_over = CHECKSUM_OVER;
		};
	public:
		TC_LFObjectPool();
		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
		// Return:
		//////////////////////////////////////////////////////////////////////////
		TC_LFObjectPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~TC_LFObjectPool();

		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.  
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA* Alloc(void);

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA* pData);

		//////////////////////////////////////////////////////////////////////////
		// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
		//
		// Parameters: ����.
		// Return: (int) �޸� Ǯ ���� ��ü ����
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacity(void) { return mCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mSize; }
		DWORD GetMallocCount() { return mMallocCount; }

	private:
		void AllocMemory(int size);

	private:
#ifdef VER_CASH_LINE
		struct alignas(64) t_Top
		{
			st_BLOCK_NODE* ptr_node = nullptr;
			LONG64 counter = 0;
		};

		alignas(64) DWORD mSize;
#else
		struct t_Top
		{
			st_BLOCK_NODE* ptr_node = nullptr;
			LONG64 counter = 0;
		};

		DWORD mSize;
#endif // VER_CASH_LINE


		DWORD mCapacity;
		DWORD mMallocCount = 0;
		bool mbPlacementNew;
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		t_Top _pFreeTop;
	};
	template<typename DATA>
	inline TC_LFObjectPool<DATA>::TC_LFObjectPool()
		: TC_LFObjectPool(0, false)
	{
	}
	template<typename DATA>
	inline TC_LFObjectPool<DATA>::TC_LFObjectPool(int iBlockNum, bool bPlacementNew)
		: mSize(0)
		, mCapacity(iBlockNum)
		, mbPlacementNew(bPlacementNew)
	{
		_pFreeTop.counter = 0;
		_pFreeTop.ptr_node = nullptr;
		AllocMemory(mCapacity);
	}
	template<typename DATA>
	inline TC_LFObjectPool<DATA>::~TC_LFObjectPool()
	{
		st_BLOCK_NODE* node = _pFreeTop.ptr_node;

		while (node != nullptr)
		{
			st_BLOCK_NODE* pNext = node->stpNextBlock;

#ifdef VER_CASH_LINE
			_aligned_free(node);
#else
			free(node);
#endif // VER_CASH_LINE


			node = pNext;
		}
	}
	template<typename DATA>
	inline DATA* TC_LFObjectPool<DATA>::Alloc(void)
	{
		alignas(16) t_Top top;
		st_BLOCK_NODE* ret;
		st_BLOCK_NODE* next;

		InterlockedIncrement(&mSize);

		if (mSize > mCapacity)
		{
			InterlockedIncrement(&mCapacity);
			AllocMemory(1);
		}

		do
		{
			top.counter = _pFreeTop.counter;
			top.ptr_node = _pFreeTop.ptr_node;
			next = top.ptr_node->stpNextBlock;
		} while (InterlockedCompareExchange128((LONG64*)&_pFreeTop, _pFreeTop.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

		ret = top.ptr_node;

		if (mbPlacementNew)
		{
			new (&ret->data) (DATA);
		}

		return &ret->data;
	}
	template<typename DATA>
	inline bool TC_LFObjectPool<DATA>::Free(DATA* pData)
	{
		// prerequisite
		st_BLOCK_NODE* top;
		//st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)((char*)pData - sizeof(st_BLOCK_NODE::code) * 2);
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)pData;

		if (pNode->code != this ||
			pNode->checkSum_over != CHECKSUM_OVER)
		{
			CRASH();
			return false;
		}

		do
		{
			top = _pFreeTop.ptr_node;
			pNode->stpNextBlock = top;
		} while (InterlockedCompareExchangePointer((PVOID*)&_pFreeTop, pNode, top) != top);

		if (mbPlacementNew)
		{
			pData->~DATA();
		}

		InterlockedDecrement(&mSize);
		return true;
	}
	template<typename DATA>
	inline void TC_LFObjectPool<DATA>::AllocMemory(int size)
	{
		//alignas(16) t_Top top;
		st_BLOCK_NODE* top;
		st_BLOCK_NODE* node = nullptr;


		for (int i = 0; i < size; ++i)
		{
			// prerequisite
#ifdef VER_CASH_LINE
			node = (st_BLOCK_NODE*)_aligned_malloc(sizeof(st_BLOCK_NODE), 64);
#else
			node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
#endif // VER_CASH_LINE
			InterlockedIncrement(&mMallocCount);
			node->code = this;
			new (&node->data) (DATA);
			node->checkSum_over = CHECKSUM_OVER;

			do
			{
				top = _pFreeTop.ptr_node;
				node->stpNextBlock = top;
			} while (InterlockedCompareExchangePointer((PVOID*)&_pFreeTop, node, top) != top);
		}
	}
}

