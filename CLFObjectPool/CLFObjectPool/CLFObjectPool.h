#pragma once
#include <new.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>

#define CHECKSUM_UNDER (0xAAAAAAAA)
#define CHECKSUM_OVER (0xBBBBBBBB)

namespace procademy
{
	class CLFObjectPool
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
			ULONG64 data;
			st_BLOCK_NODE* stpNextBlock;
			unsigned int checkSum_over = CHECKSUM_OVER;
		};
	public:
		CLFObjectPool();
		//////////////////////////////////////////////////////////////////////////
		// 생성자, 파괴자.
		//
		// Parameters:	(int) 초기 블럭 개수.
		//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CLFObjectPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~CLFObjectPool();

		//////////////////////////////////////////////////////////////////////////
		// 블럭 하나를 할당받는다.  
		//
		// Parameters: 없음.
		// Return: (DATA *) 데이타 블럭 포인터.
		//////////////////////////////////////////////////////////////////////////
		ULONG64* Alloc(void);

		//////////////////////////////////////////////////////////////////////////
		// 사용중이던 블럭을 해제한다.
		//
		// Parameters: (DATA *) 블럭 포인터.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(ULONG64* pData);

		//////////////////////////////////////////////////////////////////////////
		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
		//
		// Parameters: 없음.
		// Return: (int) 메모리 풀 내부 전체 개수
		//////////////////////////////////////////////////////////////////////////
		int		GetCapacity(void) { return mCapacity; }

		//////////////////////////////////////////////////////////////////////////
		// 현재 사용중인 블럭 개수를 얻는다.
		//
		// Parameters: 없음.
		// Return: (int) 사용중인 블럭 개수.
		//////////////////////////////////////////////////////////////////////////
		int		GetSize(void) { return mSize; }

	private:
		void AllocMemory(int size);

	private:
		struct t_Top
		{
			st_BLOCK_NODE* ptr_node = nullptr;
			LONG64 counter = 0;
		};

		DWORD mSize;
		DWORD mCapacity;
		bool mbPlacementNew;
		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
		alignas(16) t_Top _pFreeTop;
	};
}