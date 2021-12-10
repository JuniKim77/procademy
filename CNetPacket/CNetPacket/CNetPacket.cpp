#include "CNetPacket.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include "CProfiler.h"
#include "CLogger.h"
#include "CCrashDump.h"

#define NET_VERSION
//#define TEST

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
	{
		mBuffer = (char*)malloc(mCapacity + HEADER_MAX_SIZE);
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mBuffer + HEADER_MAX_SIZE;
		mZero = mBuffer;
		mEnd = mFront + mCapacity;
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
		mFront = mBuffer + HEADER_MAX_SIZE;
		mRear = mFront;
	}

	int CNetPacket::MoveFront(DWORD iSize)
	{
		if (mFront + iSize > mRear)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"MoveFront - front is over rear");

			iSize = mRear - mFront;
		}

		mFront += iSize;

		return iSize;
	}

	int CNetPacket::MoveRear(DWORD iSize)
	{
		if (mRear + iSize >= mEnd)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"MoveFront - front is over rear");

			iSize = mEnd - mRear;
		}

		mRear += iSize;

		return iSize;
	}

	CNetPacket& CNetPacket::operator<<(unsigned char byValue)
	{
		if (sizeof(unsigned char) + mRear > mEnd)
		{
			resize();
		}

		*mRear = byValue;
		mRear++;

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(char chValue)
	{
		if (sizeof(char) + mRear > mEnd)
		{
			resize();
		}

		*mRear = chValue;
		mRear++;

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(short shValue)
	{
		if (sizeof(short) + mRear > mEnd)
		{
			resize();
		}

		*((short*)mRear) = shValue;
		mRear += sizeof(short);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned short wValue)
	{
		if (sizeof(unsigned short) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned short*)mRear) = wValue;
		mRear += sizeof(unsigned short);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(int iValue)
	{
		if (sizeof(int) + mRear > mEnd)
		{
			resize();
		}

		*((int*)mRear) = iValue;
		mRear += sizeof(int);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned int iValue)
	{
		if (sizeof(unsigned int) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned int*)mRear) = iValue;
		mRear += sizeof(unsigned int);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(long lValue)
	{
		if (sizeof(long) + mRear > mEnd)
		{
			resize();
		}

		*((long*)mRear) = lValue;
		mRear += sizeof(long);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(unsigned long lValue)
	{
		if (sizeof(unsigned long) + mRear > mEnd)
		{
			resize();
		}

		*((unsigned long*)mRear) = lValue;
		mRear += sizeof(unsigned long);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(float fValue)
	{
		if (sizeof(float) + mRear > mEnd)
		{
			resize();
		}

		*((float*)mRear) = fValue;
		mRear += sizeof(float);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(__int64 iValue)
	{
		if (sizeof(__int64) + mRear > mEnd)
		{
			resize();
		}

		*((__int64*)mRear) = iValue;
		mRear += sizeof(__int64);

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(double dValue)
	{
		if (sizeof(double) + mRear > mEnd)
		{
			resize();
		}

		*((double*)mRear) = dValue;
		mRear += sizeof(double);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned char& byValue)
	{
		byValue = *((unsigned char*)mFront);
		mFront += sizeof(unsigned char);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(char& chValue)
	{
		chValue = *((char*)mFront);
		mFront += sizeof(char);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(short& shValue)
	{
		shValue = *((short*)mFront);
		mFront += sizeof(short);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned short& wValue)
	{
		wValue = *((unsigned short*)mFront);
		mFront += sizeof(unsigned short);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(int& iValue)
	{
		iValue = *((int*)mFront);
		mFront += sizeof(int);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned int& iValue)
	{
		iValue = *((unsigned int*)mFront);
		mFront += sizeof(unsigned int);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(long& dwValue)
	{
		dwValue = *((long*)mFront);
		mFront += sizeof(long);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(unsigned long& dwValue)
	{
		dwValue = *((unsigned long*)mFront);
		mFront += sizeof(unsigned long);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(float& fValue)
	{
		fValue = *((float*)mFront);
		mFront += sizeof(float);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(__int64& iValue)
	{
		iValue = *((__int64*)mFront);
		mFront += sizeof(__int64);

		return *this;
	}

	CNetPacket& CNetPacket::operator>>(double& dValue)
	{
		dValue = *((double*)mFront);
		mFront += sizeof(double);

		return *this;
	}

	int CNetPacket::GetData(char* chpDest, int iLength)
	{
		if (mFront + iLength > mRear)
		{
			iLength = mRear - mFront;
		}

		memcpy(chpDest, mFront, iLength);
		mFront += iLength;

		return iLength;
	}

	int CNetPacket::GetData(wchar_t* chpDest, int iLength)
	{
		return GetData((char*)chpDest, iLength * 2);
	}

	int CNetPacket::PutData(const char* chpSrc, int iLength)
	{
		if (iLength + mRear > mEnd)
		{
			resize();
		}

		memcpy(mRear, chpSrc, iLength);
		mRear += iLength;

		return iLength;
	}

	int CNetPacket::PutData(const wchar_t* chpSrc, int iLength)
	{
		return PutData((const char*)chpSrc, iLength * 2);
	}

	CNetPacket& CNetPacket::operator<<(const char* s)
	{
		PutData(s, strlen(s));

		return *this;
	}

	CNetPacket& CNetPacket::operator<<(const wchar_t* s)
	{
		PutData(s, wcslen(s));

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
		int ret = (int)InterlockedDecrement16((SHORT*)&mRefCount);

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

	void CNetPacket::SetHeader(bool isLengthOnly)
	{
		if (isLengthOnly)
		{
			mZero = mBuffer + (HEADER_MAX_SIZE - 2);
			*((USHORT*)mZero) = (USHORT)(mRear - mFront);
		}
		else
		{
			st_Header header;

			header.code = sCode;
			header.len = (USHORT)(mRear - mFront);
#ifdef TEST
			header.randKey = 0x31;
#else
			header.randKey = (BYTE)rand();
#endif // TEST

			char* pFront = mFront;
			BYTE sum = 0;

			for (int i = 0; i < header.len; ++i)
			{
				sum += (BYTE)*pFront;
				pFront++;
			}

			header.checkSum = sum;

			mZero = mBuffer;
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

		BYTE len = mRear - mFront + 1;

		for (BYTE i = 1; i <= len; ++i)
		{
			D = (BYTE)*pFront;
			P = D ^ (P + header.randKey + i);
			E = P ^ (E + sPacketKey + i);

			*pFront = (char)E;

			pFront++;
		}
	}

	bool CNetPacket::Decode()
	{
		char* pFront = mFront - 1;
		BYTE curP;
		BYTE prevP = 0;
		BYTE curE;
		BYTE prevE = 0;
		BYTE D;
		st_Header header = *((st_Header*)mZero);

		BYTE len = mRear - mFront + 1;

		for (BYTE i = 1; i <= len; ++i)
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

		for (int i = 1; i < len; ++i)
		{
			sum += (BYTE)*pFront;
			pFront++;
		}

		header = *((st_Header*)mZero);

		return sum == header.checkSum;
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
		char* pBuffer = (char*)malloc((long long)mCapacity + HEADER_MAX_SIZE + eBUFFER_DEFAULT);

		if (pBuffer != 0)
			memcpy(pBuffer, mBuffer, mCapacity + HEADER_MAX_SIZE);

		int frontIndex = (int)(mFront - mBuffer);
		int rearIndex = (int)(mRear - mBuffer);

		Release();

		mBuffer = pBuffer;

		mFront = mBuffer + frontIndex;
		mRear = mBuffer + rearIndex;

		mCapacity += eBUFFER_DEFAULT;

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Resize");
	}

	void CNetPacket::writeBuffer(const char* src, int size)
	{
		memcpy(mRear, src, size);
		MoveRear(size);
	}
}
