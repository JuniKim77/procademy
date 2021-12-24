#include "CLanPacket.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include "CProfiler.h"
#include "CLogger.h"
#include "CCrashDump.h"

//#define TEST

namespace procademy
{
	//#define DEBUG
#ifdef MEMORY_POOL_VER
	alignas(64) TC_LFObjectPool<CLanPacket> CLanPacket::sPacketPool;
#elif defined(TLS_MEMORY_POOL_VER)
	alignas(64) ObjectPool_TLS<CLanPacket> CLanPacket::sPacketPool;
#endif // MEMORY_POOL_VER

	CLanPacket::CLanPacket()
		: CLanPacket(eBUFFER_DEFAULT)
	{
	}

	CLanPacket::CLanPacket(int iBufferSize)
		: mCapacity(iBufferSize)
	{
		mBuffer = (char*)malloc((LONGLONG)mCapacity + HEADER_MAX_SIZE);
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mBuffer + HEADER_MAX_SIZE;
		mEnd = mFront + mCapacity;
		mZero = mBuffer + (HEADER_MAX_SIZE - sizeof(SHORT));
	}

	CLanPacket::~CLanPacket()
	{
		Release();
	}

	void CLanPacket::Release(void)
	{
		if (mBuffer != nullptr)
			free(mBuffer);
	}

	void CLanPacket::Clear(void)
	{
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mFront;
	}

	int CLanPacket::MoveFront(int iSize)
	{
		if (mFront + iSize > mRear)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"MoveFront - front is over rear");

			iSize = (int)(mRear - mFront);
		}

		mFront += iSize;

		return iSize;
	}

	int CLanPacket::MoveRear(int iSize)
	{
		if (mRear + iSize >= mEnd)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"MoveFront - front is over rear");

			iSize = (int)(mEnd - mRear);
		}

		mRear += iSize;

		return iSize;
	}

	CLanPacket& CLanPacket::operator<<(unsigned char byValue)
	{
		if (sizeof(unsigned char) + mRear > mEnd)
		{
			resize();
		}

		*mRear = byValue;
		mRear++;

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(char chValue)
	{
		if (sizeof(char) + mRear > mEnd)
		{
			resize();
		}

		*mRear = chValue;
		mRear++;

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(short shValue)
	{
		if (sizeof(short) + mRear > mEnd)
		{
			resize();
		}

		*((short*)mRear) = shValue;
		mRear += sizeof(short);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned short wValue)
	{
		if (sizeof(unsigned short) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned short*)mRear) = wValue;
		mRear += sizeof(unsigned short);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(int iValue)
	{
		if (sizeof(int) + mRear > mEnd)
		{
			resize();
		}

		*((int*)mRear) = iValue;
		mRear += sizeof(int);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned int iValue)
	{
		if (sizeof(unsigned int) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned int*)mRear) = iValue;
		mRear += sizeof(unsigned int);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(long lValue)
	{
		if (sizeof(long) + mRear > mEnd)
		{
			resize();
		}

		*((long*)mRear) = lValue;
		mRear += sizeof(long);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned long lValue)
	{
		if (sizeof(unsigned long) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned long*)mRear) = lValue;
		mRear += sizeof(unsigned long);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(float fValue)
	{
		if (sizeof(float) + mRear > mEnd)
		{
			resize();
		}

		*((float*)mRear) = fValue;
		mRear += sizeof(float);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(__int64 iValue)
	{
		if (sizeof(__int64) + mRear > mEnd)
		{
			resize();
		}

		*((__int64*)mRear) = iValue;
		mRear += sizeof(__int64);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(double dValue)
	{
		if (sizeof(double) + mRear > mEnd)
		{
			resize();
		}

		*((double*)mRear) = dValue;
		mRear += sizeof(double);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned char& byValue)
	{
		byValue = *((unsigned char*)mFront);
		mFront += sizeof(unsigned char);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(char& chValue)
	{
		chValue = *((char*)mFront);
		mFront += sizeof(char);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(short& shValue)
	{
		shValue = *((short*)mFront);
		mFront += sizeof(short);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned short& wValue)
	{
		wValue = *((unsigned short*)mFront);
		mFront += sizeof(unsigned short);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(int& iValue)
	{
		iValue = *((int*)mFront);
		mFront += sizeof(int);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned int& iValue)
	{
		iValue = *((unsigned int*)mFront);
		mFront += sizeof(unsigned int);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(long& dwValue)
	{
		dwValue = *((long*)mFront);
		mFront += sizeof(long);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned long& dwValue)
	{
		dwValue = *((unsigned long*)mFront);
		mFront += sizeof(unsigned long);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(float& fValue)
	{
		fValue = *((float*)mFront);
		mFront += sizeof(float);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(__int64& iValue)
	{
		iValue = *((__int64*)mFront);
		mFront += sizeof(__int64);

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(double& dValue)
	{
		dValue = *((double*)mFront);
		mFront += sizeof(double);

		return *this;
	}

	int CLanPacket::GetData(char* chpDest, int iLength)
	{
		if (mFront + iLength > mRear)
		{
			iLength = (int)(mRear - mFront);
		}

		memcpy(chpDest, mFront, iLength);
		mFront += iLength;

		return iLength;
	}

	int CLanPacket::GetData(wchar_t* chpDest, int iLength)
	{
		return GetData((char*)chpDest, iLength * 2);
	}

	int CLanPacket::PutData(const char* chpSrc, int iLength)
	{
		if (iLength + mRear > mEnd)
		{
			resize();
		}

		memcpy(mRear, chpSrc, iLength);
		mRear += iLength;

		return iLength;
	}

	int CLanPacket::PutData(const wchar_t* chpSrc, int iLength)
	{
		return PutData((const char*)chpSrc, iLength * 2);
	}

	CLanPacket& CLanPacket::operator<<(const char* s)
	{
		PutData(s, (int)strlen(s));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(const wchar_t* s)
	{
		PutData(s, (int)wcslen(s));

		return *this;
	}

	CLanPacket* CLanPacket::AllocAddRef()
	{
#ifdef PROFILE
		CProfiler::Begin(L"AllocAddRef");
#endif

		CLanPacket* ret;
#ifdef NEW_DELETE_VER
		ret = new CLanPacket;
#elif defined(MEMORY_POOL_VER)
		ret = sPacketPool.Alloc();
#elif defined(TLS_MEMORY_POOL_VER)
		ret = sPacketPool.Alloc();
#endif // NEW_DELETE_VER

		ret->mRefCount = 1;

#ifdef PROFILE
		CProfiler::End(L"AllocAddRef");
#endif

		return ret;
	}

	void CLanPacket::AddRef()
	{
		InterlockedIncrement16((SHORT*)&mRefCount);
	}

	void CLanPacket::SubRef()
	{
		int ret = InterlockedDecrement16((SHORT*)&mRefCount);

		if (ret < 0)
		{
			CRASH();
		}

		if (ret == 0)
		{
			Clear();

#ifdef PROFILE
			CProfiler::Begin(L"Free");
#endif

#ifdef NEW_DELETE_VER
			delete this;
#elif defined(MEMORY_POOL_VER)
			sPacketPool.Free(this);
#elif defined(TLS_MEMORY_POOL_VER)
			sPacketPool.Free(this);
#endif // NEW_DELETE_VER

#ifdef PROFILE
			CProfiler::End(L"Free");
#endif
		}
	}

	void CLanPacket::SetHeader()
	{
		*((USHORT*)mZero) = (USHORT)(mRear - mFront);
	}

	int CLanPacket::GetPoolCapacity()
	{
#ifdef MEMORY_POOL_VER
		return CLanPacket::sPacketPool.GetCapacity();
#elif defined(TLS_MEMORY_POOL_VER)
		return CLanPacket::sPacketPool.GetCapacity();
#else
		return 0;
#endif // MEMORY_POOL_VER
	}

	DWORD CLanPacket::GetPoolSize()
	{
#ifdef MEMORY_POOL_VER
		return CLanPacket::sPacketPool.GetSize();
#elif defined(TLS_MEMORY_POOL_VER)
		return CLanPacket::sPacketPool.GetSize();
#else
		return 0;
#endif // MEMORY_POOL_VER
	}

	void CLanPacket::resize()
	{
		char* pBuffer = (char*)malloc((long long)mCapacity + HEADER_MAX_SIZE + eBUFFER_DEFAULT);

		if (pBuffer != 0)
			memcpy(pBuffer, mBuffer, (LONGLONG)mCapacity + HEADER_MAX_SIZE);

		int frontIndex = (int)(mFront - mBuffer);
		int rearIndex = (int)(mRear - mBuffer);

		Release();

		mBuffer = pBuffer;

		mFront = mBuffer + frontIndex;
		mRear = mBuffer + rearIndex;
		mZero = mBuffer + (HEADER_MAX_SIZE - sizeof(short));

		mCapacity += eBUFFER_DEFAULT;

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Resize");
	}

	void CLanPacket::writeBuffer(const char* src, int size)
	{
		memcpy(mRear, src, size);
		MoveRear(size);
	}
}
