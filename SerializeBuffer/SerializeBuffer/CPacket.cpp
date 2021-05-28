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

int CPacket::MoveWritePos(int iSize)
{

	return 0;
}

int CPacket::MoveReadPos(int iSize)
{
	if (iSize <= 0)
		return 0;

	int useSize = mRear - mFront;

	return 0;
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

	writeBuffer(&byValue, sizeof(unsigned char));

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

	writeBuffer(&shValue, sizeof(short));

	return *this;
}

CPacket& CPacket::operator<<(unsigned short wValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator<<(int iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator<<(long lValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator<<(float fValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator<<(__int64 iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator<<(double dValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(unsigned char& byValue)
{
	readBuffer(&byValue, sizeof(unsigned char));

	return *this;
}

CPacket& CPacket::operator>>(char& chValue)
{
	readBuffer(&chValue, sizeof(char));

	return *this;
}

CPacket& CPacket::operator>>(short& shValue)
{
	readBuffer(&shValue, sizeof(short));

	return *this;
}

CPacket& CPacket::operator>>(unsigned short& wValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(int& iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(unsigned int& dwValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(float& fValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(__int64& iValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

CPacket& CPacket::operator>>(double& dValue)
{
	// TODO: 여기에 return 문을 삽입합니다.
}

void CPacket::resize()
{
	char* pBuffer = (char*)malloc(mCapacity + eBUFFER_DEFAULT);

	memcpy(pBuffer, mBuffer, mSize);

	int frontIndex = mFront - mBuffer;
	int rearIndex = mRear - mBuffer;

	Release();

	mBuffer = pBuffer;

	mFront = mBuffer + frontIndex;
	mRear = mBuffer + rearIndex;

	mCapacity += eBUFFER_DEFAULT;
}

void CPacket::writeBuffer(const void* src, int size)
{
	memcpy(mRear, src, size);
	mRear += size;
	mSize += size;
}

void CPacket::readBuffer(void* dest, int size)
{
	memcpy(mFront, dest, size);

	mFront += size;
	mSize -= size;
}
