#include "CLFObjectPool.h"
#include "CCrashDump.h"

LONG64 g_null_counter = 0;

procademy::CLFObjectPool::CLFObjectPool()
	: CLFObjectPool(0, false)
{
}

procademy::CLFObjectPool::CLFObjectPool(int iBlockNum, bool bPlacementNew)
	: mSize(0)
	, mCapacity(iBlockNum)
	, mbPlacementNew(bPlacementNew)
{
	_pFreeTop.counter = 0;
	_pFreeTop.ptr_node = nullptr;
	AllocMemory(mCapacity);
}

procademy::CLFObjectPool::~CLFObjectPool()
{
	st_BLOCK_NODE* node = _pFreeTop.ptr_node;

	int count = 0;

	while (node != nullptr)
	{
		st_BLOCK_NODE* pNext = node->stpNextBlock;
		free(node);
		node = pNext;
		count++;
	}
}

ULONG64* procademy::CLFObjectPool::Alloc(void)		// pop
{
	//DWORD incresedSize = InterlockedIncrement(&mSize);
	//InterlockedIncrement(&mSize);

	InterlockedIncrement(&mSize);

	if (mSize > mCapacity)
	{
		InterlockedIncrement(&mCapacity);
		AllocMemory(1);
	}

	alignas(16) t_Top top;
	st_BLOCK_NODE* ret = nullptr;
	st_BLOCK_NODE* next = nullptr;

	do
	{
		top.ptr_node = _pFreeTop.ptr_node;
		top.counter = _pFreeTop.counter;
		ret = top.ptr_node;
		next = top.ptr_node->stpNextBlock;
	} while (InterlockedCompareExchange128((LONG64*)&_pFreeTop, top.counter + 1, (LONG64)next, (LONG64*)&top) == 0);
	//if (_pFreeTop.counter == g_null_counter + 1)
	//{
	//	CDebugger::_Log(L"After After AllocMemory(2), but NULL [%08d]", _pFreeTop.counter);
	//}

	if (mbPlacementNew)
	{
		new (&ret->data) (ULONG64);
	}

	return &ret->data;
}

bool procademy::CLFObjectPool::Free(ULONG64* pData)		// Push
{
	// prerequisite
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
	} while (InterlockedCompareExchangePointer((PVOID*)&_pFreeTop, pNode, top) != top);

	if (mbPlacementNew)
	{
		pData->~ULONG64();
	}

	InterlockedDecrement(&mSize);
	return true;
}

void procademy::CLFObjectPool::AllocMemory(int size) // push
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
			new (&node->data) (ULONG64);
		}
		node->checkSum_over = CHECKSUM_OVER;
		node->data = 0x0000000055555555;

		//do
		//{
		//	top.ptr_node = _pFreeTop.ptr_node;
		//	top.counter = _pFreeTop.counter;
		//	node->stpNextBlock = (st_BLOCK_NODE*)top.ptr_node;
		//} while (InterlockedCompareExchange128((LONG64*)&_pFreeTop, top.counter + 1, (LONG64)node, (LONG64*)&top) == 0);

		do
		{
			top = _pFreeTop.ptr_node;
			node->stpNextBlock = (st_BLOCK_NODE*)top;
		} while (InterlockedCompareExchangePointer((PVOID*)&_pFreeTop, node, top) != top);
	}
}
