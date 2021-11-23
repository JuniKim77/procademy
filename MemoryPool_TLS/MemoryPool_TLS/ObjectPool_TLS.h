#pragma once
#include "TC_LFObjectPool.h"
#include <wtypes.h>

struct packetDebug;

extern USHORT g_debugIdx;
extern packetDebug g_packetDebugs[USHRT_MAX + 1];
extern void packetLog(
	int			logicId,
	DWORD		threadId,
	void*		pChunk,
	void*		pPacket,
	int			allocCount,
	LONG		freeCount
);

#define CHUNK_CHECKSUM (0xBBBBBBBB)

namespace procademy
{
	template <typename DATA>
	class ObjectPool_TLS
	{
	private:
		struct st_Chunk_Block;
		struct st_FreeCount;

		class CChunk
		{
		public:
			DATA* Alloc(void);
			bool	Free(DATA* pData);

			enum {
				MAX_SIZE = 1000
			};

			st_Chunk_Block				mArray[MAX_SIZE];
			ObjectPool_TLS*				pObjPool = nullptr;
			int							mAllocCount = 0;
			alignas(64) LONG			mFreeCount;
			DWORD						threadID;
		};

		struct st_Chunk_Block
		{
			DATA			data;
			void*			code;
			CChunk*			pOrigin;
			unsigned int	checkSum_over = CHUNK_CHECKSUM;
		};

	public:
		ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
		virtual	~ObjectPool_TLS();
		DATA*	Alloc(void);
		bool	Free(DATA* pData);
		int		GetCapacity(void);
		DWORD	GetSize(void) { return mSize; }
		void	OnOffCounting() { mbSizeCheck = !mbSizeCheck; }

	private:
		TC_LFObjectPool<CChunk>*	mMemoryPool;
		DWORD						mSize;
		bool						mbSizeCheck;
		DWORD						mIndex;
		CChunk*						chunkTrack[USHRT_MAX + 1];
		USHORT						trackIdx = 0;
	};

	template<typename DATA>
	inline ObjectPool_TLS<DATA>::ObjectPool_TLS(bool bPlacementNew, bool sizeCheck)
		: mSize(0)
		, mbSizeCheck(sizeCheck)
	{
		//mMemoryPool = new TC_LFObjectPool<CChunk>(0, bPlacementNew);
		mMemoryPool = (TC_LFObjectPool<CChunk>*)_aligned_malloc(sizeof(TC_LFObjectPool<CChunk>), 64);
		new (mMemoryPool) TC_LFObjectPool<CChunk>;
		mIndex = TlsAlloc();
	}
	template<typename DATA>
	inline ObjectPool_TLS<DATA>::~ObjectPool_TLS()
	{
		if (mMemoryPool != nullptr)
		{
			//delete mMemoryPool;
			_aligned_free(mMemoryPool);
		}
	}
	template<typename DATA>
	inline DATA* ObjectPool_TLS<DATA>::Alloc(void)
	{
		CChunk* chunk = (CChunk*)TlsGetValue(mIndex);

		if (chunk == nullptr || chunk->threadID != GetCurrentThreadId() || chunk->mAllocCount == CChunk::MAX_SIZE)
		{
			chunk = mMemoryPool->Alloc();
			//packetLog(30040, GetCurrentThreadId(), chunk, nullptr, chunk->mAllocCount, chunk->mFreeCount);
			chunk->threadID = GetCurrentThreadId();
			chunk->pObjPool = this;
			chunk->mAllocCount = 0;
			chunk->mFreeCount = 0;
			TlsSetValue(mIndex, chunk);

			//USHORT ret = InterlockedIncrement16((SHORT*)&trackIdx);
			//chunkTrack[ret] = chunk;
		}

		if (mbSizeCheck)
		{
			InterlockedIncrement((LONG*)&mSize);
		}

		return chunk->Alloc();
	}
	template<typename DATA>
	inline bool ObjectPool_TLS<DATA>::Free(DATA* pData)
	{
		st_Chunk_Block* block = (st_Chunk_Block*)pData;

		block->pOrigin->Free(pData);

		if (mbSizeCheck)
		{
			InterlockedDecrement((LONG*)&mSize);
		}

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
		st_Chunk_Block* pBlock = (st_Chunk_Block*)&mArray[mAllocCount++];

		//packetLog(20000, GetCurrentThreadId(), this, pBlock, mAllocCount, mFreeCount);
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

		LONG ret = InterlockedIncrement(&mFreeCount);
		//packetLog(40020, GetCurrentThreadId(), this, block, mAllocCount, ret);

		if (ret == CChunk::MAX_SIZE)
		{
			//packetLog(40040, GetCurrentThreadId(), this, block, mAllocCount, ret);
			block->pOrigin->threadID = 0;
			pObjPool->mMemoryPool->Free(block->pOrigin);
		}

		return true;
	}
}
