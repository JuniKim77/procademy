#include "CLFObjectPool.h"
#include "CCrashDump.h"

procademy::CLFObjectPool::CLFObjectPool(int iBlockNum, bool bPlacementNew)
	: mSize(0)
	, mCapacity(iBlockNum)
	, mbPlacementNew(bPlacementNew)
{
	AllocMemory(mCapacity);
}

procademy::CLFObjectPool::~CLFObjectPool()
{
	st_BLOCK_NODE* node = _pFreeTop.ptr_node;

	while (node != nullptr)
	{
		st_BLOCK_NODE* pNext = node->stpNextBlock;
		free(node);
		node = pNext;
	}
}

ULONG64* procademy::CLFObjectPool::Alloc(void)
{
	alignas(16) t_Top top;
	st_BLOCK_NODE* node;
	st_BLOCK_NODE* ret;
	st_BLOCK_NODE* next;

	if ((int)InterlockedIncrement(&mSize) > mCapacity)
	{
		AllocMemory(1);
		InterlockedIncrement(&mCapacity);
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
		new (&ret->data) (ULONG64);
	}

	return &ret->data;
}

bool procademy::CLFObjectPool::Free(ULONG64* pData)
{
	// prerequisite
	alignas(8) void* top;
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
		pData->~ULONG64();
	}

	InterlockedDecrement(&mSize);
	return true;
}

void procademy::CLFObjectPool::AllocMemory(int size)
{
	alignas(8) void* top;
	st_BLOCK_NODE* node = nullptr;

	for (int i = 0; i < size; ++i)
	{
		// prerequisite
		node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
		node->checkSum_under = CHECKSUM_UNDER;
		node->code = this;
		if (mbPlacementNew)
		{
			new (&node->data) (ULONG64);
		}
		node->checkSum_over = CHECKSUM_OVER;

		do
		{
			top = _pFreeTop.ptr_node;
			node->stpNextBlock = (st_BLOCK_NODE*)top;
		} while (InterlockedCompareExchange64((LONG64*)&_pFreeTop, (LONG64)node, (LONG64)top) != (LONG64)top);
	}
}
