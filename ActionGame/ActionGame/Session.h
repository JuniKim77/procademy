#pragma once
//#define SERVER_PORT (5000)
#define SERVER_PORT (20000)
#define WM_SOCKET (WM_USER+1)

//#include <WinSock2.h>
#include "RingBuffer.h"
//#include "PacketDefine.h"
typedef UINT_PTR        SOCKET;
struct stHeader;
class BaseObject;
class CPacket;

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
	void sendTCP(int size);
	void ReceivePacket();
	void receiveTCP(int size);
	void recvProc();
	void readMessage(stHeader* header);
	void ErrorQuit(const WCHAR* msg, const WCHAR* fileName, unsigned int lineNum);
	void ErrorDisplay(const WCHAR* msg);
	bool IsConnected() { return mbConnected; }

	// Contents
	void CreateMyPlayer(CPacket* packet);
	void CreateOtherPlayer(CPacket* packet);
	void DeletePlayer(CPacket* packet);
	void MoveStartPlayer(CPacket* packet);
	void MoveStopPlayer(CPacket* packet);
	void AttackProc1(CPacket* packet);
	void AttackProc2(CPacket* packet);
	void AttackProc3(CPacket* packet);
	void DamageProc(CPacket* packet);
	void SyncProc(CPacket* packet);

protected:
	BaseObject* SearchObject(int id);

private:
	SOCKET mSocket;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	bool mbConnected = false;
};