#include "CNetSmartPacket.h"
#include <windows.h>
#include "CNetPacket.h"

procademy::CNetSmartPacket::CNetSmartPacket(CNetPacket* packet)
{
	mpMemory = packet;

	if (mpRefCount == nullptr)
	{
		long* temp = new long;
		*temp = 0;
		mpRefCount = temp;
	}

	InterlockedIncrement(mpRefCount);
}

procademy::CNetSmartPacket::CNetSmartPacket(const CNetSmartPacket& packet)
{
	mpMemory = packet.mpMemory;
	mpRefCount = packet.mpRefCount;

	InterlockedIncrement(mpRefCount);
}

procademy::CNetSmartPacket::~CNetSmartPacket()
{
	if (mpRefCount == nullptr)
		return;

	if (InterlockedDecrement(mpRefCount) == 0)
	{
		mpMemory->SubRef();
		delete mpRefCount;

		mpMemory = nullptr;
		mpRefCount = nullptr;
	}
}

void procademy::CNetSmartPacket::operator=(const CNetSmartPacket& packet)
{
	mpMemory = packet.mpMemory;
	mpRefCount = packet.mpRefCount;

	InterlockedIncrement(mpRefCount);
}

void procademy::CNetSmartPacket::operator=(CNetPacket* packet)
{
	mpMemory = packet;

	if (mpRefCount == nullptr)
	{
		long* temp = new long;
		*temp = 0;
		mpRefCount = temp;
	}

	InterlockedIncrement(mpRefCount);
}
