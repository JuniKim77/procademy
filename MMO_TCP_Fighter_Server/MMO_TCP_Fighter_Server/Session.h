#pragma once

#include <wtypes.h>
#include <unordered_map>
//#include "RingBuffer.h"
#include "CStreamQ.h"
#include <stack>
#include <vector>

struct stHeader;
class CPacket;

class Session
{
	friend void NetWorkProc(); 
	friend void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
	friend void AcceptProc();

public:
	Session(SOCKET socket, u_short port, u_long ip, DWORD sessionNo);
	~Session();
	void SetDisconnect() { mbAlive = false; }
	void printInfo() const;
	// void setLogin() { mbLogin = true; }
	/// <summary>
	/// 받은 패킷을 리시브 링버퍼에 넣는 함수
	/// </summary>
	void receiveProc();
	/// <summary>
	/// 센드 링버퍼에 메시지를 넣어주는 함수
	/// </summary>
	void sendPacket(char* buffer, int size);
	bool completeRecvPacket();
	void writeProc();
	DWORD GetSessionNo() { return mSessionNo; }
	ULONGLONG GetLastRecvTime() { return mLastRecvTime; }
	void SetLastRecvTime(ULONGLONG time) { mLastRecvTime = time; }
	bool IsAlive() { return mbAlive; }

private:
	SOCKET mSocket;
	u_short mPort;
	u_long mIP;
	CStreamQ mSendBuffer;
	CStreamQ mRecvBuffer;
	DWORD mSessionNo;
	ULONGLONG mLastRecvTime;
	// bool mbLogin = false;
	bool mbAlive = true;
};