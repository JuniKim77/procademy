#pragma once
#pragma warning(disable: 6011)

#include <new.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include "CCrashDump.h"
//#include "myNewMalloc.h"

//#define SAFE_MODE

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

#ifdef SAFE_MODE
			void* code;
			alignas(64) DATA				data;
			st_BLOCK_NODE* stpNextBlock;
			unsigned int					checkSum_over = CHECKSUM_OVER;
#else
			DATA				data;
			st_BLOCK_NODE* stpNextBlock;
#endif // SAFE_MODE

			
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

	private:
		void AllocMemory(int size);

	private:
		struct alignas(64) t_Top
		{
			st_BLOCK_NODE*	ptr_node = nullptr;
			LONG64			counter = 0;
		};

		alignas(64) long	mSize;
		alignas(64) long	mCapacity;
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

#ifdef SAFE_MODE
			_aligned_free(node);
#else
			free(node);
#endif // SAFE_MODE
			node = pNext;
		}
	}
	template<typename DATA>
	inline DATA* TC_LFObjectPool<DATA>::Alloc(void)
	{
		alignas(16) t_Top top;
		st_BLOCK_NODE* ret;
		st_BLOCK_NODE* next;

		long capa = mCapacity;

		if (capa < InterlockedIncrement(&mSize))
		{
			AllocMemory(1);

			InterlockedIncrement(&mCapacity);
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


#ifdef SAFE_MODE
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)((char*)pData - 64);

		if (pNode->code != this ||
			pNode->checkSum_over != CHECKSUM_OVER)
		{
			CRASH();
			return false;
		}
#else
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)pData;
#endif // SAFE_MODE

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
#ifdef SAFE_MODE
			node = (st_BLOCK_NODE*)_aligned_malloc(sizeof(st_BLOCK_NODE), 64);
			node->code = this;
			node->checkSum_over = CHECKSUM_OVER;
#else
			node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
#endif // SAFE_MODE

			if (false == mbPlacementNew)
			{
				new (&node->data) (DATA);
			}

			do
			{
				top = _pFreeTop.ptr_node;
				node->stpNextBlock = top;
			} while (InterlockedCompareExchangePointer((PVOID*)&_pFreeTop, node, top) != top);
		}
	}
}
