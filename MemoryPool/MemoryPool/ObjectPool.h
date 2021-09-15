/*---------------------------------------------------------------
	procademy MemoryPool.

	�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

	MemPool.Free(pData);
----------------------------------------------------------------*/
#ifndef  __PROCADEMY_OBJECT_POOL__
#define  __PROCADEMY_OBJECT_POOL__
#include <new.h>
#include <stdlib.h>
#include <string.h>

#define CHECKSUM_UNDER (0xAAAAAAAA)
#define CHECKSUM_OVER (0xBBBBBBBB)

namespace procademy
{
	template <typename DATA>
	class ObjectPool
	{
	private:
		/* **************************************************************** */
		// �� �� �տ� ���� ��� ����ü.
		/* **************************************************************** */
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
		ObjectPool();
		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
		// Return:
		//////////////////////////////////////////////////////////////////////////
		ObjectPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~ObjectPool();

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
		int mSize;
		int mCapacity;
		bool mbPlacementNew;
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		st_BLOCK_NODE* _pFreeNode;
	};
	template<typename DATA>
	inline ObjectPool<DATA>::ObjectPool()
		: ObjectPool(0, false)
	{
	}
	template<typename DATA>
	inline ObjectPool<DATA>::ObjectPool(int iBlockNum, bool bPlacementNew)
		: mSize(0)
		, mCapacity(iBlockNum)
		, mbPlacementNew(bPlacementNew)
	{
		AllocMemory(mCapacity);
	}
	template<typename DATA>
	inline ObjectPool<DATA>::~ObjectPool()
	{
		while (_pFreeNode != nullptr)
		{
			st_BLOCK_NODE* pNext = _pFreeNode->stpNextBlock;
			free(_pFreeNode);
			_pFreeNode = pNext;
		}
	}
	template<typename DATA>
	inline DATA* ObjectPool<DATA>::Alloc(void)
	{
		if (_pFreeNode == nullptr)
		{
			AllocMemory(1);
			mCapacity++;
		}

		st_BLOCK_NODE* node = _pFreeNode;
		_pFreeNode = _pFreeNode->stpNextBlock;
		mSize++;

		if (mbPlacementNew)
		{
			new (&node->data) (DATA);
		}

		return &node->data;
	}
	template<typename DATA>
	inline bool ObjectPool<DATA>::Free(DATA* pData)
	{
		st_BLOCK_NODE* pNode = (st_BLOCK_NODE*)((char*)pData - sizeof(st_BLOCK_NODE::code) * 2);

		if (pNode->code != this ||
			pNode->checkSum_under != CHECKSUM_UNDER ||
			pNode->checkSum_over != CHECKSUM_OVER)
		{
			return false;
		}
		if (mbPlacementNew)
		{
			pData->~DATA();
		}

		pNode->stpNextBlock = _pFreeNode;
		_pFreeNode = pNode;
		mSize--;
		return true;
	}
	template<typename DATA>
	inline void ObjectPool<DATA>::AllocMemory(int size)
	{
		st_BLOCK_NODE* node = nullptr;

		for (int i = 0; i < size; ++i)
		{
			node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
			node->checkSum_under = CHECKSUM_UNDER;
			node->code = this;
			if (mbPlacementNew)
			{
				new (&node->data) (DATA);
			}
			node->stpNextBlock = _pFreeNode;
			node->checkSum_over = CHECKSUM_OVER;

			_pFreeNode = node;
		}
	}
}
#endif