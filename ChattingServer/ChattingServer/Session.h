#pragma once

#include <wtypes.h>
#include <unordered_map>
#include "RingBuffer.h"

struct st_PACKET_HEADER;
class CPacket;

class Session
{
	friend void NetWorkProc(); 
	friend void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
	
public:
	Session(SOCKET socket, u_short port, u_long ip, DWORD sessionNo);
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
	bool receiveProc();
	void writePacket();
	void sendPacket(char* buffer, int size);

private:
	bool readMessage(st_PACKET_HEADER* header);
	BYTE makeCheckSum(CPacket* packet, WORD msgType);
	void ReceiveHelper(int size);
	void WriteHelper(int size);

private:
	SOCKET mSocket;
	u_short mPort;
	u_long mIP;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	DWORD mSessionNo;
	bool mbLogin = false;
	bool mbAlive = true;
};