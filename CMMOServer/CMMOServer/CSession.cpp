#include "CSession.h"

procademy::CSession::CSession()
{
	ZeroMemory(&recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&sendOverlapped, sizeof(WSAOVERLAPPED));
}

procademy::CSession::~CSession()
{
}
