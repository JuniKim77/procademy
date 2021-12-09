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
	BYTE	CLanPacket::sCode = 0;
	BYTE	CLanPacket::sPacketKey = 0;

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
		, mPacketSize(0)
		, mHeaderSize(HEADER_MAX_SIZE)
	{
		mBuffer = (char*)malloc(mCapacity);
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mBuffer + HEADER_MAX_SIZE;

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
		mPacketSize = 0;
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mFront;
		mZero = mBuffer;
	}

	int CLanPacket::MoveFront(int iSize)
	{
		if (iSize <= 0)
			return 0;

		int size = iSize <= mPacketSize ? iSize : mPacketSize;

		mFront += size;
		mPacketSize -= size;

		return size;
	}

	int CLanPacket::MoveRear(int iSize)
	{
		if (iSize <= 0)
			return 0;

		int size = iSize <= GetFreeSize() ? iSize : GetFreeSize();

		mRear += size;
		mPacketSize += size;

		return size;
	}

	CLanPacket& CLanPacket::operator<<(unsigned char byValue)
	{
		if (sizeof(unsigned char) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&byValue, sizeof(unsigned char));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(char chValue)
	{
		if (sizeof(char) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer(&chValue, sizeof(char));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(short shValue)
	{
		if (sizeof(short) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&shValue, sizeof(short));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned short wValue)
	{
		if (sizeof(unsigned short) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&wValue, sizeof(unsigned short));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(int iValue)
	{
		if (sizeof(int) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(int));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned int iValue)
	{
		if (sizeof(unsigned int) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(unsigned int));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(long lValue)
	{
		if (sizeof(long) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&lValue, sizeof(long));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(unsigned long lValue)
	{
		if (sizeof(unsigned long) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&lValue, sizeof(unsigned long));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(float fValue)
	{
		if (sizeof(float) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&fValue, sizeof(float));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(__int64 iValue)
	{
		if (sizeof(__int64) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(__int64));

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(double dValue)
	{
		if (sizeof(double) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&dValue, sizeof(double));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned char& byValue)
	{
		unsigned char* pBuf = (unsigned char*)mFront;

		byValue = *pBuf;
		MoveFront(sizeof(unsigned char));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(char& chValue)
	{
		char* pBuf = mFront;

		chValue = *pBuf;
		MoveFront(sizeof(char));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(short& shValue)
	{
		short* pBuf = (short*)mFront;

		shValue = *pBuf;
		MoveFront(sizeof(short));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned short& wValue)
	{
		unsigned short* pBuf = (unsigned short*)mFront;

		wValue = *pBuf;
		MoveFront(sizeof(unsigned short));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(int& iValue)
	{
		int* pBuf = (int*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(int));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned int& iValue)
	{
		unsigned int* pBuf = (unsigned int*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(unsigned int));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(long& dwValue)
	{
		long* pBuf = (long*)mFront;

		dwValue = *pBuf;
		MoveFront(sizeof(long));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(unsigned long& dwValue)
	{
		unsigned long* pBuf = (unsigned long*)mFront;

		dwValue = *pBuf;
		MoveFront(sizeof(unsigned long));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(float& fValue)
	{
		float* pBuf = (float*)mFront;

		fValue = *pBuf;
		MoveFront(sizeof(float));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(__int64& iValue)
	{
		__int64* pBuf = (__int64*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(__int64));

		return *this;
	}

	CLanPacket& CLanPacket::operator>>(double& dValue)
	{
		double* pBuf = (double*)mFront;

		dValue = *pBuf;
		MoveFront(sizeof(double));

		return *this;
	}

	int CLanPacket::GetData(char* chpDest, int iLength)
	{
		int size = iLength >= mPacketSize ? mPacketSize : iLength;

		memcpy(chpDest, mFront, size);
		MoveFront(size);

		return size;
	}

	int CLanPacket::GetData(wchar_t* chpDest, int iLength)
	{
		return GetData((char*)chpDest, iLength * 2);
	}

	int CLanPacket::PutData(const char* chpSrc, int iLength)
	{
		if (iLength > GetFreeSize())
		{
			resize();

			// log
		}

		memcpy(mRear, chpSrc, iLength);
		MoveRear(iLength);

		return iLength;
	}

	int CLanPacket::PutData(const wchar_t* chpSrc, int iLength)
	{
		return PutData((const char*)chpSrc, iLength * 2);
	}

	CLanPacket& CLanPacket::operator<<(const char* s)
	{
		int len = (int)strlen(s);

		PutData(s, len);

		return *this;
	}

	CLanPacket& CLanPacket::operator<<(const wchar_t* s)
	{
		int len = (int)wcslen(s);

		PutData(s, len);

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
		char* pBuffer = (char*)malloc((long long)mCapacity + eBUFFER_DEFAULT);

		if (pBuffer != 0)
			memcpy(pBuffer, mBuffer, mPacketSize);

		int frontIndex = (int)(mFront - mBuffer);
		int rearIndex = (int)(mRear - mBuffer);

		Release();

		mBuffer = pBuffer;

		mFront = mBuffer + frontIndex;
		mRear = mBuffer + rearIndex;

		mCapacity += eBUFFER_DEFAULT;
	}

	void CLanPacket::writeBuffer(const char* src, int size)
	{
		memcpy(mRear, src, size);
		MoveRear(size);
	}
}
