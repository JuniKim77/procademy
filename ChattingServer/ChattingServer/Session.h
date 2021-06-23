#pragma once

#include <wtypes.h>
#include <unordered_map>
#include "RingBuffer.h"

class Session
{
	friend void NetWorkProc(); 
	friend void DestroySessionProc();
	friend void SelectProc(std::unordered_map<DWORD, Session*>::iterator iter, FD_SET* rset);
public:
	Session(SOCKET socket, u_short port, u_long ip);
	~Session();
	void SetDisconnect() { mbAlive = false; }
	void printInfo() const;
	void setLogin() { mbLogin = true; }
	/// <summary>
	/// 받은 패킷을 리시브 링버퍼에 넣는 함수
	/// </summary>
	void receivePacket();
	/// <summary>
	/// 리시브 링버퍼에서 패킷을 꺼내와서 로직 처리하는 함수
	/// </summary>
	void receiveProc();

private:
	void ReceiveHelper(int size);

private:
	SOCKET mSocket;
	u_short mPort;
	u_long mIP;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	bool mbLogin = false;
	bool mbAlive = true;
};