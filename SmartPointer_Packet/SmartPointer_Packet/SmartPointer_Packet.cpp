#include "SmartPointer_Packet.h"

CSmartPointer_Packet::CSmartPointer_Packet(CPacket* packet)
{
    mpMemory = packet;

    if (mpRefCount == nullptr)
    {
        int* temp = new int;
        *temp = 0;
        mpRefCount = temp;
    }
    (*mpRefCount)++;
}

CSmartPointer_Packet::CSmartPointer_Packet(const CSmartPointer_Packet& packet)
{
    mpMemory = packet.mpMemory;
    mpRefCount = packet.mpRefCount;

    (*mpRefCount)++;
}

CSmartPointer_Packet::~CSmartPointer_Packet()
{
    (*mpRefCount)--;

    if (mpRefCount == 0)
    {
        delete mpMemory;
        delete mpRefCount;

        mpMemory = nullptr;
        mpRefCount = nullptr;
    }
}

void CSmartPointer_Packet::operator=(const CSmartPointer_Packet& packet)
{
    mpMemory = packet.mpMemory;
    mpRefCount = packet.mpRefCount;

    (*mpRefCount)++;
}

void CSmartPointer_Packet::operator=(CPacket* packet)
{
    mpMemory = packet;

    if (mpRefCount == nullptr)
    {
        int* temp = new int;
        *temp = 0;
        mpRefCount = temp;
    }
    (*mpRefCount)++;
}

CPacket* CSmartPointer_Packet::operator*()
{
    return mpMemory;
}
