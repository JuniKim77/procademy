#pragma once
#include "TC_LFObjectPool.h"
#include <wtypes.h>

#define ALLOC_CHECK_VER

struct packetDebug;

//extern USHORT g_debugIdx;
//extern packetDebug g_packetDebugs[USHRT_MAX + 1];
//extern void packetLog(
//	int			logicId,
//	DWORD		threadId,
//	void*		pChunk,
//	void*		pPacket,
//	int			allocCount,
//	LONG		freeCount
//);

#define CHUNK_CHECKSUM (0xBBBBBBBB)

namespace procademy
{
	template <typename DATA>
	class ObjectPool_TLS
	{
		friend class CChunk;

	private:
		struct st_Element;
		struct st_FreeCount;

		class CChunk
		{
		public:
			DATA*	Alloc(void);
			bool	Free(DATA* pData);

			enum {
				MAX_SIZE = 1000
			};

			st_Element					mArray[MAX_SIZE];
			int							mAllocCount = 0;
			DWORD						threadID;
			ObjectPool_TLS*				pObjPool = nullptr;			
			alignas(64) LONG			mFreeCount;
		};

		struct st_Element
		{
			DATA			data;
			CChunk*			pOrigin;
			void*			code;
		};

	public:
		ObjectPool_TLS(bool bPlacementNew = false, bool sizeCheck = false);
		virtual	~ObjectPool_TLS();
		DATA*	Alloc(void);
		void	Free(DATA* pData);
		int		GetCapacity(void);
		DWORD	GetSize(void);
		void	OnOffCounting() { mbSizeCheck = !mbSizeCheck; }

		TC_LFObjectPool<CChunk>*	mMemoryPool;
	private:
		alignas(64) DWORD			mSize;
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
			_aligned_free(mMemoryPool);
		}
	}
	template<typename DATA>
	inline DATA* ObjectPool_TLS<DATA>::Alloc(void)
	{
		CChunk* chunk = (CChunk*)TlsGetValue(mIndex);
		DWORD myID = GetCurrentThreadId();

		if (chunk == nullptr || chunk->mAllocCount == CChunk::MAX_SIZE || chunk->threadID != myID)
		{
			chunk = mMemoryPool->Alloc();
			//packetLog(30040, GetCurrentThreadId(), chunk, nullptr, chunk->mAllocCount, chunk->mFreeCount);
			chunk->threadID = myID;
			chunk->pObjPool = this;
			chunk->mAllocCount = 0;
			chunk->mFreeCount = 0;
			TlsSetValue(mIndex, chunk);

			/*USHORT ret = InterlockedIncrement16((SHORT*)&trackIdx);
			chunkTrack[ret] = chunk;*/
		}

#ifdef ALLOC_CHECK_VER
		if (mbSizeCheck)
		{
			InterlockedIncrement((LONG*)&mSize);
		}
#endif // ALLOC_CHECK_VER
		return chunk->Alloc();
	}
	template<typename DATA>
	inline void ObjectPool_TLS<DATA>::Free(DATA* pData)
	{
		st_Element* block = (st_Element*)pData;

		block->pOrigin->Free(pData);
#ifdef ALLOC_CHECK_VER
		if (mbSizeCheck)
		{
			InterlockedDecrement((LONG*)&mSize);
		}
#endif // ALLOC_CHECK_VER
	}
	template<typename DATA>
	inline int ObjectPool_TLS<DATA>::GetCapacity(void)
	{
		return mMemoryPool->GetCapacity() * CChunk::MAX_SIZE;
	}
	template<typename DATA>
	inline DWORD ObjectPool_TLS<DATA>::GetSize(void)
	{
#ifdef ALLOC_CHECK_VER
		if (mbSizeCheck)
		{
			return mSize;
		}
#endif // ALLOC_CHECK_VER
		return mMemoryPool->GetSize();
	}
	template<typename DATA>
	inline DATA* ObjectPool_TLS<DATA>::CChunk::Alloc(void)
	{
		st_Element* pBlock = (st_Element*)&mArray[mAllocCount++];

		//packetLog(20000, GetCurrentThreadId(), this, pBlock, mAllocCount, mFreeCount);
		pBlock->pOrigin = this;
		pBlock->code = pObjPool;
		//pBlock->checkSum_over = CHUNK_CHECKSUM;

		return (DATA*)pBlock;
	}
	template<typename DATA>
	inline bool ObjectPool_TLS<DATA>::CChunk::Free(DATA* pData)
	{
		st_Element* block = (st_Element*)pData;

		if (block->code != pObjPool)
		{
			CRASH();
			return false;
		}

		if (InterlockedIncrement(&mFreeCount) == CChunk::MAX_SIZE)
		{
			//packetLog(40040, GetCurrentThreadId(), this, block, mAllocCount, ret);
			block->pOrigin->threadID = 0;
			pObjPool->mMemoryPool->Free(block->pOrigin);
		}

		return true;
	}
}
