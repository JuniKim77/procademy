#pragma once
#define SERVER_PORT (5000)
#define WM_SOCKET (WM_USER+1)

#include <WinSock2.h>
#include "RingBuffer.h"

class Session
{
public:
	friend void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	Session();
	~Session();
	bool Connect(HWND hWnd);
	void Disconnect();
	void SendPacket(char* packet, int size);
	void writeProc();
	void ReceivePacket();
	void recvProc();
	void ErrorQuit(const WCHAR* msg);
	void ErrorDisplay(const WCHAR* msg);	
	bool IsConnected() { return mbConnected; }

private:
	SOCKET mSocket;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	bool mbConnected = false;
};