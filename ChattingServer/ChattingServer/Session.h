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
	/// ���� ��Ŷ�� ���ú� �����ۿ� �ִ� �Լ�
	/// </summary>
	void receivePacket(); // �Ϸ�
	/// <summary>
	/// ���ú� �����ۿ��� ��Ŷ�� �����ͼ� ���� ó���ϴ� �Լ�
	/// </summary>
	void sendPacket(char* buffer, int size); // �ϼ�
	bool receiveProc(); // �ϼ�
	void writeProc();

private:
	bool readMessage(st_PACKET_HEADER* header); // �Ϸ�
	BYTE makeCheckSum(CPacket* packet, WORD msgType);
	bool ReceiveHelper(int size); // �ϼ�
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
	std::stack<WORD> mCheckSums;
	std::stack<WORD> mRecvSizes;
	std::vector<st_PACKET_HEADER> mRecvHeaders;
	int mSendCount = 0;
};