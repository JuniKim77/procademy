/*---------------------------------------------------------------
	procademy MemoryPool.

	메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
	특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.

	- 사용법.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData 사용

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
		// 각 블럭 앞에 사용될 노드 구조체.
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
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		ObjectPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~ObjectPool();

		//////////////////////////////////////////////////////////////////////////
		// 블럭 하나를 할당받는다.  
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		DATA* Alloc(void);

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA* pData);

		//////////////////////////////////////////////////////////////////////////
		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		//
		// Parameters: 없음.
		// Return: (int) 메모리 풀 내부 전체 개수
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacity(void) { return mSize; }

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 블럭 개수를 얻는다.
		//
		// Parameters: 없음.
		// Return: (int) 사용중인 블럭 개수.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mCapacity; }

	private:
		void* AllocMemory(int size);

	private:
		int mSize;
		int mCapacity;
		bool mbPlacementNew;
		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
		st_BLOCK_NODE* _pFreeNode;
	};
	template<typename DATA>
	inline ObjectPool<DATA>::ObjectPool(int iBlockNum, bool bPlacementNew)
		: mSize(0)
		, mCapacity(iBlockNum)
		, mbPlacementNew(bPlacementNew)
	{
		_pFreeNode = (st_BLOCK_NODE*)AllocMemory(mCapacity);
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
			_pFreeNode = (st_BLOCK_NODE*)AllocMemory(mCapacity);
			mCapacity *= 2;
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
	inline void* ObjectPool<DATA>::AllocMemory(int size)
	{
		st_BLOCK_NODE* node = nullptr;
		st_BLOCK_NODE* nodePost = nullptr;

		if (mbPlacementNew)
		{
			for (int i = 0; i < size; ++i)
			{
				node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
				node->checkSum_under = CHECKSUM_UNDER;
				node->code = this;
				node->stpNextBlock = nodePost;
				node->checkSum_over = CHECKSUM_OVER;

				nodePost = node;
			}
		}
		else
		{
			for (int i = 0; i < size; ++i)
			{
				node = (st_BLOCK_NODE*)malloc(sizeof(st_BLOCK_NODE));
				node->checkSum_under = CHECKSUM_UNDER;
				node->code = this;
				new (&node->data) (DATA);
				node->stpNextBlock = nodePost;
				node->checkSum_over = CHECKSUM_OVER;

				nodePost = node;
			}
		}

		return node;
	}
}
#endif