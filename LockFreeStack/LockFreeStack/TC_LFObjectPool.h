#pragma once
#include <new.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include "CCrashDump.h"
#include "CDebugger.h"

#define CHECKSUM_UNDER (0xAAAAAAAA)
#define CHECKSUM_OVER (0xBBBBBBBB)

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

			unsigned int checkSum_under = CHECKSUM_UNDER;
			void* code;
			DATA data;
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
		int		GetMallocSize(void) { return mMallocCount; }

	private:
		void AllocMemory(int size);

	private:
		struct t_Top
		{
			st_BLOCK_NODE* ptr_node = nullptr;
			LONG64 counter = 0;
		};
		struct st_Debug
		{
			DWORD size;
			DWORD capa;
		};

		DWORD mSize;
		DWORD mCapacity;
		bool mbPlacementNew;
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		DWORD mMallocCount = 0;
		alignas(16) t_Top _pFreeTop;
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
			free(node);
			node = pNext;
		}
	}
	template<typename DATA>
	inline DATA* TC_LFObjectPool<DATA>::Alloc(void)
	{
		alignas(16) t_Top top;
		st_BLOCK_NODE* ret;
		st_BLOCK_NODE* next;
		st_Debug values[3];

		//values[0].size = mSize;
		//values[0].capa = mCapacity;
		int retVal = InterlockedIncrement(&mSize);
		//values[1].size = retVal;
		//values[1].capa = mCapacity;
		//values[2].size = mSize;
		//values[2].capa = mCapacity;
		if (mSize > mCapacity)
		{
			AllocMemory(1);
			InterlockedIncrement(&mCapacity);

			if (_pFreeTop.ptr_node == nullptr)
			{
				CRASH();
				CDebugger::_Log(L"After AllocMemory(2), but NULL [%08d]", _pFreeTop.counter);
			}
		}

		do
		{
			top.ptr_node = _pFreeTop.ptr_node;
			top.counter = _pFreeTop.counter;
			ret = top.ptr_node;
			next = top.ptr_node->stpNextBlock;
		} while (InterlockedCompareExchange128((LONG64*)&_pFreeTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);

		if (mbPlacementNew)
		{
			new (&ret->data) (DATA);
		}

		/*CDebugger::_Log(L"Inc [%5u][%5u]->[%5d][%5u]->[%5d][%5d]",
			values[0].size, values[0].capa,
			values[1].size, values[1].capa,
			values[2].size, values[2].capa);*/

		return &ret->data;
	}
	template<typename DATA>
	inline bool TC_LFObjectPool<DATA>::Free(DATA* pData)
	{
		// prerequisite
		st_Debug values[3];
		void* top;
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)((char*)pData - sizeof(st_BLOCK_NODE::code) * 2);

		if (pNode->code != this ||
			pNode->checkSum_under != CHECKSUM_UNDER ||
			pNode->checkSum_over != CHECKSUM_OVER)
		{
			CRASH();
			return false;
		}

		do
		{
			top = _pFreeTop.ptr_node;
			pNode->stpNextBlock = (st_BLOCK_NODE*)top;
		} while (InterlockedCompareExchange64((LONG64*)&_pFreeTop, (LONG64)pNode, (LONG64)top) != (LONG64)top);

		if (mbPlacementNew)
		{
			pData->~DATA();
		}

		if (mSize > mCapacity)
		{
			AllocMemory(1);
			InterlockedIncrement(&mCapacity);

			if (_pFreeTop.ptr_node == nullptr)
			{
				CRASH();
				CDebugger::_Log(L"After AllocMemory(2), but NULL [%08d]", _pFreeTop.counter);
			}
		}

		/*values[0].size = mSize;
		values[0].capa = mCapacity;
		int retVal = InterlockedDecrement(&mSize);
		values[1].size = retVal;
		values[1].capa = mCapacity;
		values[2].size = mSize;
		values[2].capa = mCapacity;

		CDebugger::_Log(L"Dec [%5u][%5u]->[%5d][%5u]->[%5d][%5d]",
			values[0].size, values[0].capa,
			values[1].size, values[1].capa,
			values[2].size, values[2].capa);*/
		InterlockedDecrement(&mSize);

		return true;
	}
	template<typename DATA>
	inline void TC_LFObjectPool<DATA>::AllocMemory(int size)
	{
		//alignas(16) t_Top top;
		void* top;
		st_BLOCK_NODE* node = nullptr;

		for (int i = 0; i < size; ++i)
		{
			// prerequisite
			node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			InterlockedIncrement(&mMallocCount);
			node->checkSum_under = CHECKSUM_UNDER;
			node->code = this;
			if (mbPlacementNew)
			{
				new (&node->data) (DATA);
			}
			node->checkSum_over = CHECKSUM_OVER;

			/*do
			{
				top.ptr_node = _pFreeTop.ptr_node;
				top.counter = _pFreeTop.counter;
				node->stpNextBlock = (st_BLOCK_NODE*)top.ptr_node;
			} while (InterlockedCompareExchange128((LONG64*)&_pFreeTop, top.counter + 1, (LONG64)node, (LONG64*)&top) == 0);*/
			do
			{
				top = _pFreeTop.ptr_node;
				node->stpNextBlock = (st_BLOCK_NODE*)top;
			} while (InterlockedCompareExchange64((LONG64*)&_pFreeTop, (LONG64)node, (LONG64)top) != (LONG64)top);
		}
	}
}
