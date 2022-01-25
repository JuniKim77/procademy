#include "CSession.h"
#include "CNetPacket.h"

procademy::CSession::CSession()
{
	ZeroMemory(&recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&sendOverlapped, sizeof(WSAOVERLAPPED));
}

procademy::CSession::~CSession()
{
}

void procademy::CSession::SendPacket(CNetPacket* packet)
{
	packet->AddRef();

	packet->ReadySend();

	sendQ.Enqueue(packet);
}
