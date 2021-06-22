#pragma once

#include <wtypes.h>
#include "RingBuffer.h"

class Session
{
	friend void AcceptProc();
	friend void NetWorkProc();
public:
	Session(SOCKET socket, u_short port, u_long ip, DWORD idNum);
	~Session();
	void SetName(const WCHAR* name);
	void printInfo() const;
	void setLogin() { mbLogin = true; }

private:
	SOCKET mSocket;
	u_short mPort;
	u_long mIP;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	bool mbLogin = false;

	// content
	DWORD mIDNum;
	WCHAR mName[15];
};