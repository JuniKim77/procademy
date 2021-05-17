#include "session.h"
#include <iostream>
#include <WS2tcpip.h>
#include "RingBuffer.h"

Session::Session()
	: socket(INVALID_SOCKET)
	, port(0)
	, IP(0)
	, sendRingBuffer(new RingBuffer)
	, receiveRingBuffer(new RingBuffer)
{
}

Session::~Session()
{
	if (sendRingBuffer != nullptr)
		delete sendRingBuffer;

	if (receiveRingBuffer != nullptr)
		delete receiveRingBuffer;
}

void Session::printInfo() const
{
	WCHAR szIP[16];
	InetNtop(AF_INET, &IP, szIP, 16);
	wprintf_s(L"ID: %d, IP: %s, Port: %d, X: %d, Y: %d\n",
		ID, szIP, port, x, y);
}
