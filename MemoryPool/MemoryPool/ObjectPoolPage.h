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
#ifndef  __PROCADEMY_OBJECT_POOL_PAGE__
#define  __PROCADEMY_OBJECT_POOL_PAGE__
#include <new.h>
#include <stdlib.h>
#include <string.h>

#define CHECKSUM_UNDER (0xAAAAAAAA)
#define CHECKSUM_OVER (0xBBBBBBBB)

namespace procademy
{
	template <typename DATA>
	class ObjectPoolPage
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
		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
		// Return:
		//////////////////////////////////////////////////////////////////////////
		ObjectPoolPage(int iBlockNum, bool bPlacementNew = false);
		virtual	~ObjectPoolPage();

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
		st_BLOCK_NODE* mHeadPage;
		st_BLOCK_NODE* mTailPage;
	};
	template<typename DATA>
	inline ObjectPoolPage<DATA>::ObjectPoolPage(int iBlockNum, bool bPlacementNew)
		: mSize(0)
		, mCapacity(iBlockNum)
		, mBlockUnit(iBlockNum)
		, mbPlacementNew(bPlacementNew)
	{
		mHeadPage = (st_BLOCK_NODE*)AllocMemory();
		mTailPage = mHeadPage;
		_pFreeNode = (st_BLOCK_NODE*)mHeadPage;
	}
	template<typename DATA>
	inline ObjectPoolPage<DATA>::~ObjectPoolPage()
	{
		st_BLOCK_NODE* pCur = mHeadPage;

		while (1)
		{
			if (pCur == nullptr)
			{
				break;
			}
#ifdef _WIN64
			long long** pAddress = (long long**)(pCur + mBlockUnit);
#else
			int** pAddress = (int**)(pCur + mBlockUnit);
#endif
			if (!mbPlacementNew)
			{
				st_BLOCK_NODE* node = pCur;

				for (int i = 0; i < mBlockUnit; ++i)
				{
					(node->data).~DATA();
					node++;
				}
			}

			free(pCur);
			pCur = (st_BLOCK_NODE*)*pAddress;
		}
	}
	template<typename DATA>
	inline DATA* ObjectPoolPage<DATA>::Alloc(void)
	{
		if (_pFreeNode == nullptr) 
		{
			_pFreeNode = (st_BLOCK_NODE*)AllocMemory();
#ifdef _WIN64
			long long** pAddress = (long long**)(mTailPage + mBlockUnit);
			*pAddress = (long long*)_pFreeNode;
#else
			int** pAddress = (int**)(mTailPage + mBlockUnit);
			*pAddress = (int*)_pFreeNode;
#endif
			mTailPage = (st_BLOCK_NODE*)*pAddress;
			mCapacity += mBlockUnit;
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
	inline bool ObjectPoolPage<DATA>::Free(DATA* pData)
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
	inline void* ObjectPoolPage<DATA>::AllocMemory()
	{
		void* retMemory = malloc(sizeof(st_BLOCK_NODE) * mBlockUnit + sizeof(st_BLOCK_NODE*));
		st_BLOCK_NODE* pOrigin = (st_BLOCK_NODE*)retMemory;
		st_BLOCK_NODE* node = pOrigin;

		if (mbPlacementNew)
		{
			for (int i = 0; i < mBlockUnit; ++i)
			{
				node->checkSum_under = CHECKSUM_UNDER;
				node->code = this;
				node->stpNextBlock = node + 1;
				node->checkSum_over = CHECKSUM_OVER;
				node++;
			}
		}
		else
		{
			for (int i = 0; i < mBlockUnit; ++i)
			{
				node->checkSum_under = CHECKSUM_UNDER;
				node->code = this;
				new (&node->data) (DATA);
				node->stpNextBlock = node + 1;
				node->checkSum_over = CHECKSUM_OVER;
				node++;
			}
		}
		(node - 1)->stpNextBlock = nullptr;

#ifdef _WIN64
		long long* pAddress = (long long*)(pOrigin + mBlockUnit);
		*pAddress = NULL;
#else
		int* pAddress = (int*)(pOrigin + mBlockUnit);
		*pAddress = NULL;
#endif
		return retMemory;
	}
}
#endif