#include "CNetPacket.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include "CProfiler.h"
#include "CLogger.h"
#include "CCrashDump.h"

#define NET_VERSION
//#define TEST
//#define PROFILE

namespace procademy
{
	BYTE	CNetPacket::sCode = 0;
	BYTE	CNetPacket::sPacketKey = 0;

	//#define DEBUG
#ifdef MEMORY_POOL_VER
	alignas(64) TC_LFObjectPool<CNetPacket> CNetPacket::sPacketPool;
#elif defined(TLS_MEMORY_POOL_VER)
	alignas(64) ObjectPool_TLS<CNetPacket> CNetPacket::sPacketPool;
#endif // MEMORY_POOL_VER

	CNetPacket::CNetPacket()
		: CNetPacket(eBUFFER_DEFAULT)
	{
	}

	CNetPacket::CNetPacket(int iBufferSize)
		: mCapacity(iBufferSize)
		, mPacketSize(0)
		, mHeaderSize(HEADER_MAX_SIZE)
	{
		mBuffer = (char*)malloc(mCapacity);
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mBuffer + HEADER_MAX_SIZE;
#ifdef NET_VERSION
		mZero = mBuffer;
#else
		mZero = mBuffer + (HEADER_MAX_SIZE - sizeof(SHORT));
#endif // NET_VERSION
	}

	CNetPacket::~CNetPacket()
	{
		Release();
	}

	void CNetPacket::Release(void)
	{
		if (mBuffer != nullptr)
			free(mBuffer);
	}

	void CNetPacket::Clear(void)
	{
		mPacketSize = 0;
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mFront;
	}

	int CNetPacket::MoveFront(int iSize)
	{
		if (iSize <= 0)
			return 0;

		int size = iSize <= mPacketSize ? iSize : mPacketSize;

		mFront += size;
		mPacketSize -= size;

		return size;
	}

	int CNetPacket::MoveRear(int iSize)
	{
		if (iSize <= 0)
			return 0;

		int size = iSize <= GetFreeSize() ? iSize : GetFreeSize();

		mRear += size;
		mPacketSize += size;

		return size;
	}

	CNetPacket& CNetPacket::operator<<(unsigned char byValue)
	{
		if (sizeof(unsigned char) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&byValue, sizeof(unsigned char));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(char chValue)
	{
		if (sizeof(char) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer(&chValue, sizeof(char));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(short shValue)
	{
		if (sizeof(short) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&shValue, sizeof(short));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned short wValue)
	{
		if (sizeof(unsigned short) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&wValue, sizeof(unsigned short));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(int iValue)
	{
		if (sizeof(int) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(int));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned int iValue)
	{
		if (sizeof(unsigned int) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(unsigned int));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(long lValue)
	{
		if (sizeof(long) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&lValue, sizeof(long));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned long lValue)
	{
		if (sizeof(unsigned long) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&lValue, sizeof(unsigned long));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(float fValue)
	{
		if (sizeof(float) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&fValue, sizeof(float));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(__int64 iValue)
	{
		if (sizeof(__int64) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&iValue, sizeof(__int64));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(double dValue)
	{
		if (sizeof(double) > GetFreeSize())
		{
			resize();

			// log
		}

		writeBuffer((char*)&dValue, sizeof(double));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned char& byValue)
	{
		unsigned char* pBuf = (unsigned char*)mFront;

		byValue = *pBuf;
		MoveFront(sizeof(unsigned char));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(char& chValue)
	{
		char* pBuf = mFront;

		chValue = *pBuf;
		MoveFront(sizeof(char));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(short& shValue)
	{
		short* pBuf = (short*)mFront;

		shValue = *pBuf;
		MoveFront(sizeof(short));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned short& wValue)
	{
		unsigned short* pBuf = (unsigned short*)mFront;

		wValue = *pBuf;
		MoveFront(sizeof(unsigned short));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(int& iValue)
	{
		int* pBuf = (int*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(int));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned int& iValue)
	{
		unsigned int* pBuf = (unsigned int*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(unsigned int));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(long& dwValue)
	{
		long* pBuf = (long*)mFront;

		dwValue = *pBuf;
		MoveFront(sizeof(long));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned long& dwValue)
	{
		unsigned long* pBuf = (unsigned long*)mFront;

		dwValue = *pBuf;
		MoveFront(sizeof(unsigned long));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(float& fValue)
	{
		float* pBuf = (float*)mFront;

		fValue = *pBuf;
		MoveFront(sizeof(float));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(__int64& iValue)
	{
		__int64* pBuf = (__int64*)mFront;

		iValue = *pBuf;
		MoveFront(sizeof(__int64));

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(double& dValue)
	{
		double* pBuf = (double*)mFront;

		dValue = *pBuf;
		MoveFront(sizeof(double));

		return *this;
	}

	int CNetPacket::GetData(char* chpDest, int iLength)
	{
		int size = iLength >= mPacketSize ? mPacketSize : iLength;

		memcpy(chpDest, mFront, size);
		MoveFront(size);

		return size;
	}

	int CNetPacket::GetData(wchar_t* chpDest, int iLength)
	{
		return GetData((char*)chpDest, iLength * 2);
	}

	int CNetPacket::PutData(const char* chpSrc, int iLength)
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

	int CNetPacket::PutData(const wchar_t* chpSrc, int iLength)
	{
		return PutData((const char*)chpSrc, iLength * 2);
	}

	CNetPacket& CNetPacket::operator<<(const char* s)
	{
		int len = (int)strlen(s);

		PutData(s, len);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(const wchar_t* s)
	{
		int len = (int)wcslen(s);

		PutData(s, len);

		return *this;
	}

	CNetPacket* CNetPacket::AllocAddRef()
	{
#ifdef PROFILE
		CProfiler::Begin(L"AllocAddRef");
#endif

		CNetPacket* ret;
#ifdef NEW_DELETE_VER
		ret = new CNetPacket;
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

	void CNetPacket::AddRef()
	{
		InterlockedIncrement16((SHORT*)&mRefCount);
	}

	void CNetPacket::SubRef()
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

	void CNetPacket::SetHeader(bool isLengthOnly)
	{
		if (isLengthOnly)
		{
			mZero = mBuffer + (HEADER_MAX_SIZE - 2);
			mHeaderSize = 2;
			*((USHORT*)mZero) = (USHORT)mPacketSize;
		}
		else
		{
			st_Header header;

			header.code = sCode;
			header.len = (USHORT)mPacketSize;
#ifdef TEST
			header.randKey = 0x31;
#else
			header.randKey = (BYTE)rand();
#endif // TEST

			char* pFront = mFront;
			BYTE sum = 0;

			for (int i = 0; i < mPacketSize; ++i)
			{
				sum += (BYTE)*pFront;
				pFront++;
			}

			header.checkSum = sum;

			mZero = mBuffer;
			mHeaderSize = HEADER_MAX_SIZE;
			memcpy(mZero, (char*)&header, HEADER_MAX_SIZE);
		}
	}

	void CNetPacket::Encode()
	{
		char* pFront = mFront - 1;
		BYTE P = 0;
		BYTE E = 0;
		BYTE D;
		st_Header header = *((st_Header*)mZero);

		for (BYTE i = 1; i <= (BYTE)(mPacketSize + 1); ++i)
		{
			D = (BYTE)*pFront;
			P = D ^ (P + header.randKey + i);
			E = P ^ (E + sPacketKey + i);

			*pFront = (char)E;

			pFront++;
		}
	}

	void CNetPacket::Decode()
	{
		char* pFront = mFront - 1;
		BYTE curP;
		BYTE prevP = 0;
		BYTE curE;
		BYTE prevE = 0;
		BYTE D;
		st_Header header = *((st_Header*)mZero);

		for (BYTE i = 1; i <= (BYTE)(mPacketSize + 1); ++i)
		{
			curE = (BYTE)*pFront;
			curP = curE ^ (prevE + sPacketKey + i);
			D = curP ^ (prevP + header.randKey + i);

			*pFront = (char)D;
			prevE = curE;
			prevP = curP;

			pFront++;
		}

		BYTE sum = 0;
		pFront = mFront;

		for (int i = 0; i < mPacketSize; ++i)
		{
			sum += (BYTE)*pFront;
			pFront++;
		}

		header = *((st_Header*)mZero);

		if (sum != header.checkSum)
		{
			CRASH();
		}
	}

	int CNetPacket::GetPoolCapacity()
	{
#ifdef MEMORY_POOL_VER
		return CNetPacket::sPacketPool.GetCapacity();
#elif defined(TLS_MEMORY_POOL_VER)
		return CNetPacket::sPacketPool.GetCapacity();
#else
		return 0;
#endif // MEMORY_POOL_VER
	}

	DWORD CNetPacket::GetPoolSize()
	{
#ifdef MEMORY_POOL_VER
		return CNetPacket::sPacketPool.GetSize();
#elif defined(TLS_MEMORY_POOL_VER)
		return CNetPacket::sPacketPool.GetSize();
#else
		return 0;
#endif // MEMORY_POOL_VER
	}

	void CNetPacket::resize()
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

	void CNetPacket::writeBuffer(const char* src, int size)
	{
		memcpy(mRear, src, size);
		MoveRear(size);
	}
}
