#include "CPacket.h"
#include <malloc.h>
#include <string.h>

#define DEBUG

CPacket::CPacket()
	: CPacket(eBUFFER_DEFAULT)
{
}

CPacket::CPacket(int iBufferSize)
	: mCapacity(iBufferSize)
	, mSize(0)
{
	mBuffer = (char*)malloc(mCapacity);
	mFront = mBuffer;
	mRear = mBuffer;

#ifdef DEBUG
	memset(mBuffer, 0, mCapacity);
#endif
}

CPacket::~CPacket()
{
	Release();
}

void CPacket::Release(void)
{
	if (mBuffer != nullptr)
		free(mBuffer);
}

void CPacket::Clear(void)
{
	mSize = 0;
	mFront = 0;
	mRear = mFront;
}

int CPacket::MoveFront(int iSize)
{
	if (iSize <= 0)
		return 0;

	int size = iSize <= mSize ? iSize : mSize;

	mFront += size;
	mSize -= size;

	return size;
}

int CPacket::MoveRear(int iSize)
{
	if (iSize <= 0)
		return 0;

	int size = iSize <= GetFreeSize() ? iSize : GetFreeSize();

	mRear += size;
	mSize += size;

	return size;
}

int CPacket::GetFreeSize() const
{
	return mCapacity - mSize;
}

CPacket& CPacket::operator<<(unsigned char byValue)
{
	if (sizeof(unsigned char) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&byValue, sizeof(unsigned char));

	return *this;
}

CPacket& CPacket::operator<<(char chValue)
{
	if (sizeof(char) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer(&chValue, sizeof(char));

	return *this;
}

CPacket& CPacket::operator<<(short shValue)
{
	if (sizeof(short) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&shValue, sizeof(short));

	return *this;
}

CPacket& CPacket::operator<<(unsigned short wValue)
{
	if (sizeof(unsigned short) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&wValue, sizeof(unsigned short));

	return *this;
}

CPacket& CPacket::operator<<(int iValue)
{
	if (sizeof(int) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&iValue, sizeof(int));

	return *this;
}

CPacket& CPacket::operator<<(unsigned int iValue)
{
	if (sizeof(unsigned int) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&iValue, sizeof(unsigned int));

	return *this;
}

CPacket& CPacket::operator<<(long lValue)
{
	if (sizeof(long) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&lValue, sizeof(long));

	return *this;
}

CPacket& CPacket::operator<<(unsigned long lValue)
{
	if (sizeof(unsigned long) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&lValue, sizeof(unsigned long));

	return *this;
}

CPacket& CPacket::operator<<(float fValue)
{
	if (sizeof(float) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&fValue, sizeof(float));

	return *this;
}

CPacket& CPacket::operator<<(__int64 iValue)
{
	if (sizeof(__int64) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&iValue, sizeof(__int64));

	return *this;
}

CPacket& CPacket::operator<<(double dValue)
{
	if (sizeof(double) > GetFreeSize())
	{
		resize();

		// log
	}

	writeBuffer((char*)&dValue, sizeof(double));

	return *this;
}

CPacket& CPacket::operator>>(unsigned char& byValue)
{
	unsigned char* pBuf = (unsigned char*)mFront;

	byValue = *pBuf;
	MoveFront(sizeof(unsigned char));

	return *this;
}

CPacket& CPacket::operator>>(char& chValue)
{
	char* pBuf = mFront;

	chValue = *pBuf;
	MoveFront(sizeof(char));

	return *this;
}

CPacket& CPacket::operator>>(short& shValue)
{
	short* pBuf = (short*)mFront;

	shValue = *pBuf;
	MoveFront(sizeof(short));

	return *this;
}

CPacket& CPacket::operator>>(unsigned short& wValue)
{
	unsigned short* pBuf = (unsigned short*)mFront;

	wValue = *pBuf;
	MoveFront(sizeof(unsigned short));

	return *this;
}

CPacket& CPacket::operator>>(int& iValue)
{
	int* pBuf = (int*)mFront;

	iValue = *pBuf;
	MoveFront(sizeof(int));

	return *this;
}

CPacket& CPacket::operator>>(unsigned int& iValue)
{
	unsigned int* pBuf = (unsigned int*)mFront;

	iValue = *pBuf;
	MoveFront(sizeof(unsigned int));

	return *this;
}

CPacket& CPacket::operator>>(long& dwValue)
{
	long* pBuf = (long*)mFront;

	dwValue = *pBuf;
	MoveFront(sizeof(long));

	return *this;
}

CPacket& CPacket::operator>>(unsigned long& dwValue)
{
	unsigned long* pBuf = (unsigned long*)mFront;

	dwValue = *pBuf;
	MoveFront(sizeof(unsigned long));

	return *this;
}

CPacket& CPacket::operator>>(float& fValue)
{
	float* pBuf = (float*)mFront;

	fValue = *pBuf;
	MoveFront(sizeof(float));

	return *this;
}

CPacket& CPacket::operator>>(__int64& iValue)
{
	__int64* pBuf = (__int64*)mFront;

	iValue = *pBuf;
	MoveFront(sizeof(__int64));

	return *this;
}

CPacket& CPacket::operator>>(double& dValue)
{
	double* pBuf = (double*)mFront;

	dValue = *pBuf;
	MoveFront(sizeof(double));

	return *this;
}

int CPacket::GetData(char* chpDest, int iLength)
{
	int size = iLength >= mSize ? iLength : mSize;

	memcpy(chpDest, mFront, size);
	MoveFront(size);

	return size;
}

int CPacket::GetData(wchar_t* chpDest, int iLength)
{
	return GetData((char*)chpDest, iLength * 2);
}

int CPacket::PutData(const char* chpSrc, int iLength)
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

int CPacket::PutData(const wchar_t* chpSrc, int iLength)
{
	return PutData((const char*)chpSrc, iLength * 2);
}

CPacket& CPacket::operator<<(const char* s)
{
	int len = (int)strlen(s);

	PutData(s, len);

	return *this;
}

CPacket& CPacket::operator<<(const wchar_t* s)
{
	int len = (int)wcslen(s);

	PutData(s, len);

	return *this;
}

void CPacket::resize()
{
	char* pBuffer = (char*)malloc(mCapacity + eBUFFER_DEFAULT);

	memcpy(pBuffer, mBuffer, mSize);

	int frontIndex = (int)(mFront - mBuffer);
	int rearIndex = (int)(mRear - mBuffer);

	Release();

	mBuffer = pBuffer;

	mFront = mBuffer + frontIndex;
	mRear = mBuffer + rearIndex;

	mCapacity += eBUFFER_DEFAULT;
}

void CPacket::writeBuffer(const char* src, int size)
{
	memcpy(mRear, src, size);
	MoveRear(size);
}
