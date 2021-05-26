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

			int checkSum_under = 0xAAAA;
			void* code;
			DATA data;
			st_BLOCK_NODE* stpNextBlock;
			int checkSum_over = 0xBBBB;
		};

	public:
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
		int		GetCapacity(void) { return mSize; }

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mCapacity; }

	private:
		void* AllocMemory();

	private:
		int mSize;
		int mCapacity;
		int mBlockUnit;
		bool mbPlacementNew;
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		st_BLOCK_NODE* _pFreeNode;
		void* mOriginAddress;
	};
	template<typename DATA>
	inline ObjectPool<DATA>::ObjectPool(int iBlockNum, bool bPlacementNew)
		: mSize(0)
		, mCapacity(iBlockNum)
		, mBlockUnit(iBlockNum)
		, mbPlacementNew(bPlacementNew)
	{
		mOriginAddress = AllocMemory();
		_pFreeNode = (st_BLOCK_NODE*)mOriginAddress;
	}
	template<typename DATA>
	inline ObjectPool<DATA>::~ObjectPool()
	{
	}
	template<typename DATA>
	inline DATA* ObjectPool<DATA>::Alloc(void)
	{
		return NULL;
	}
	template<typename DATA>
	inline bool ObjectPool<DATA>::Free(DATA* pData)
	{
		return false;
	}
	template<typename DATA>
	inline void* ObjectPool<DATA>::AllocMemory()
	{
		void* retMemory = malloc(sizeof(st_BLOCK_NODE) * mBlockUnit + sizeof(void*));

		st_BLOCK_NODE* node = (st_BLOCK_NODE*)retMemory;

		for (int i = 0; i < mBlockUnit; ++i)
		{
			node->
			node->code = this;
		}

		if (mbPlacementNew)
			return retMemory;

		return retMemory;
	}
}
#endif