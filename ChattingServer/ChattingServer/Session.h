#pragma once

#include <wtypes.h>
#include <unordered_map>
#include "RingBuffer.h"
#include <stack>
#include <vector>

struct st_PACKET_HEADER;
class CPacket;

class Session
{
	friend bool ReqStressEcho(DWORD from, CPacket* packet);
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
	void receivePacket(); // 완료
	/// <summary>
	/// 센드 링버퍼에 메시지를 넣어주는 함수
	/// </summary>
	void sendPacket(char* buffer, int size); // 완성
	bool receiveProc(); // 완성
	void writeProc();

private:
	bool readMessage(st_PACKET_HEADER* header); // 완료
	BYTE makeCheckSum(CPacket* packet, WORD msgType);

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