#pragma once
#define SERVER_PORT (5000)
#define WM_SOCKET (WM_USER+1)

#include <WinSock2.h>
#include "RingBuffer.h"

class Session
{
public:
	Session();
	~Session();
	bool Connect(HWND hWnd);
	void SendPacket(char* packet, int size);
	void writeProc();
	void ReceivePacket(char* packet, int size);
	void recvProc();
	void ErrorQuit(const WCHAR* msg);
	void ErrorDisplay(const WCHAR* msg);	

private:
	SOCKET mSocket;
	RingBuffer sendBuffer;
	RingBuffer recvBuffer;
};