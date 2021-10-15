#pragma once

class CPacket;

class CSmartPointer_Packet
{
public:
	CSmartPointer_Packet() {}
	CSmartPointer_Packet(CPacket* packet);
	CSmartPointer_Packet(const CSmartPointer_Packet& packet);
	~CSmartPointer_Packet();
	void operator= (const CSmartPointer_Packet& packet);
	void operator= (CPacket* packet);

//private:
	CPacket* operator* ();

private:
	CPacket* mpMemory = nullptr;
	int* mpRefCount = nullptr;
};