#pragma once
#include <WinSock2.h>
#include "RingBuffer.h"

class Session
{
public:
	Session();

	SOCKET socket;
	USHORT port;
	ULONG IP;
	RingBuffer recv;
	RingBuffer send;
};