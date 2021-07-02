#pragma once

#include <wtypes.h>
#include <unordered_map>
#include "RingBuffer.h"
#include <stack>
#include <vector>

struct stHeader;
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
	// void setLogin() { mbLogin = true; }
	/// <summary>
	/// ���� ��Ŷ�� ���ú� �����ۿ� �ִ� �Լ�
	/// </summary>
	void receiveProc();
	/// <summary>
	/// ���ú� �����ۿ��� ��Ŷ�� �����ͼ� ���� ó���ϴ� �Լ�
	/// </summary>
	void sendPacket(char* buffer, int size);
	bool completeRecvPacket();
	void writeProc();

private:
	SOCKET mSocket;
	u_short mPort;
	u_long mIP;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	DWORD mSessionNo;
	// bool mbLogin = false;
	bool mbAlive = true;
};