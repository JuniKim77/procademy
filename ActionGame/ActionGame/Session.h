#pragma once
#define SERVER_PORT (5000)
#define WM_SOCKET (WM_USER+1)

//#include <WinSock2.h>
#include "RingBuffer.h"
//#include "PacketDefine.h"
typedef UINT_PTR        SOCKET;
struct stHeader;
class BaseObject;

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
	void CreateMyPlayer(const char* msg);
	void CreateOtherPlayer(const char* msg);
	void DeletePlayer(const char* msg);
	void MoveStartPlayer(const char* msg);
	void MoveStopPlayer(const char* msg);
	void AttackProc1(const char* msg);
	void AttackProc2(const char* msg);
	void AttackProc3(const char* msg);
	void DamageProc(const char* msg);

protected:
	BaseObject* SearchObject(int id);

private:
	SOCKET mSocket;
	RingBuffer mSendBuffer;
	RingBuffer mRecvBuffer;
	bool mbConnected = false;
};