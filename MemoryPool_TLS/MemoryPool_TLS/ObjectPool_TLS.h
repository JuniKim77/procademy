#include "TC_LFObjectPool.h"
#include <wtypes.h>

#define CHUNK_CHECKSUM (0xBBBBBBBB)

namespace procademy
{
	template <typename DATA>
	class ObjectPool_TLS
	{
	private:
		struct st_Chunk_Block;

		class CChunk
		{
		public:
			DATA*	Alloc(void);
			bool	Free(DATA* pData);

			enum {
				MAX_SIZE = 200
			};

			st_Chunk_Block	mArray[MAX_SIZE];
			ObjectPool_TLS* pObjPool = nullptr;
			int				mAllocCount = 0;
			int				mFreeCount = 0;
		};

		struct st_Chunk_Block
		{
			DATA	data;
			void*	code;
			CChunk* pOrigin;
			unsigned int checkSum_over = CHUNK_CHECKSUM;
		};

	public:
		ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
		virtual	~ObjectPool_TLS();
		DATA*	Alloc(void);
		bool	Free(DATA* pData);
		int		GetCapacity(void);
		DWORD	GetSize(void) { return mSize; }

	private:
		TC_LFObjectPool<CChunk>* mMemoryPool;
		DWORD mSize;
		bool mbSizeCheck;
		DWORD mIndex;
	};

	template<typename DATA>
	inline ObjectPool_TLS<DATA>::ObjectPool_TLS(bool bPlacementNew, bool sizeCheck)
		: mSize(0)
		, mbSizeCheck(sizeCheck)
	{
		mMemoryPool = new TC_LFObjectPool<CChunk>(0, bPlacementNew);
		mIndex = TlsAlloc();
	}
	template<typename DATA>
	inline ObjectPool_TLS<DATA>::~ObjectPool_TLS()
	{
		if (mMemoryPool != nullptr)
		{
			delete mMemoryPool;
		}
	}
	template<typename DATA>
	inline DATA* ObjectPool_TLS<DATA>::Alloc(void)
	{
		CChunk* chunk = (CChunk*)TlsGetValue(mIndex);

		if (chunk == nullptr || chunk->mAllocCount == CChunk::MAX_SIZE)
		{
			chunk = mMemoryPool->Alloc();

			chunk->pObjPool = this;
			chunk->mAllocCount = 0;
			chunk->mFreeCount = 0;

			TlsSetValue(mIndex, chunk);
		}

		return chunk->Alloc();
	}
	template<typename DATA>
	inline bool ObjectPool_TLS<DATA>::Free(DATA* pData)
	{
		st_Chunk_Block* block = (st_Chunk_Block*)pData;

		block->pOrigin->Free(pData);

		return true;
	}
	template<typename DATA>
	inline int ObjectPool_TLS<DATA>::GetCapacity(void)
	{
		return mMemoryPool->GetCapacity() * CChunk::MAX_SIZE;
	}
	template<typename DATA>
	inline DATA* ObjectPool_TLS<DATA>::CChunk::Alloc(void)
	{
		if (mAllocCount == MAX_SIZE)
			return nullptr;

		st_Chunk_Block* pBlock = (st_Chunk_Block*)&mArray[mAllocCount++];
		pBlock->pOrigin = this;
		pBlock->code = this;
		pBlock->checkSum_over = CHUNK_CHECKSUM;

		return (DATA*)pBlock;
	}
	template<typename DATA>
	inline bool ObjectPool_TLS<DATA>::CChunk::Free(DATA* pData)
	{
		st_Chunk_Block* block = (st_Chunk_Block*)pData;

		if (block->code != this ||
			block->checkSum_over != CHUNK_CHECKSUM)
		{
			CRASH();
			return false;
		}

		InterlockedIncrement((LONG*)&mFreeCount);

		if (mFreeCount == CChunk::MAX_SIZE)
		{
			pObjPool->mMemoryPool->Free(block->pOrigin);
		}

		return true;
	}
}