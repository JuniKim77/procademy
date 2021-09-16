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
		// ������, �ı���.
		//
		// Parameters:	(int) �ʱ� �� ����.
		//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CLFObjectPool(int iBlockNum, bool bPlacementNew = false);
		virtual	~CLFObjectPool();

		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.  
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		ULONG64* Alloc(void);

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(ULONG64* pData);

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
		struct t_Top
		{
			st_BLOCK_NODE* ptr_node = nullptr;
			LONG64 counter = 0;
		};

		DWORD mSize;
		DWORD mCapacity;
		bool mbPlacementNew;
		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
		alignas(16) t_Top _pFreeTop;
	};
}