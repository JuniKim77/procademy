#pragma comment(lib, "ws2_32")

//#define DEBUG
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include "Session.h"
#include "stdio.h"
#include "PacketDefine.h"
#include "myList.h"
#include "PlayerObject.h"
#include "EffectObject.h"
#include "ActionDefine.h"
#include "CPacket.h"
#include "CLogger.h"

extern HWND gMainWindow;
extern myList<BaseObject*> gObjectList;
extern BaseObject* gPlayerObject;
extern CLogger g_Logger;

Session::Session()
	: mSocket(INVALID_SOCKET)
{
}

Session::~Session()
{
}

bool Session::Connect(HWND hWnd)
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return false;
	}

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (mSocket == INVALID_SOCKET)
		ErrorQuit(L"Socket Create Error", __FILEW__, __LINE__);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);

#ifdef DEBUG
	
#else
	InetPton(AF_INET, L"106.245.38.107", &addr.sin_addr);
#endif // DEBUG
	/*WCHAR ServerIP[16] = { 0, };
	wprintf_s(L"Server IP: ");
	_getws_s(ServerIP);
	InetPton(AF_INET, ServerIP, &addr.sin_addr);*/

	int asyncselectRetval = WSAAsyncSelect(mSocket, hWnd, WM_SOCKET,
		FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);

	if (asyncselectRetval == SOCKET_ERROR) {
		ErrorDisplay(L"Socket Transform Error");
		return false;
	}

	int connectRetval = connect(mSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (connectRetval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			ErrorDisplay(L"Connection Error");
			return false;
		}
	}

	return true;
}

void Session::Disconnect()
{
	closesocket(mSocket);
}

void Session::SendPacket(char* packet, int size)
{
	int retval = mSendBuffer.Enqueue(packet, size);

	if (retval < size)
		ErrorQuit(L"Send Error to sendBuffer", __FILEW__, __LINE__);
}

void Session::writeProc()
{
	int sendSize = send(mSocket, mSendBuffer.GetFrontBufferPtr(), mSendBuffer.DirectDequeueSize(), 0);

	mSendBuffer.MoveFront(sendSize);

	if (sendSize == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		ErrorQuit(L"Send Error", __FILEW__, __LINE__);

		return;
	}
}

void Session::ReceivePacket()
{
	/*int dSize = mRecvBuffer.DirectEnqueueSize();
	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), dSize, 0);

	mRecvBuffer.MoveRear(retval);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		ErrorQuit(L"Receive Error to recv buffer", __FILEW__, __LINE__);

		return;
	}*/

	WSABUF wsabufs[2];

	int dSize = mRecvBuffer.DirectEnqueueSize();

	wsabufs[0].buf = mRecvBuffer.GetRearBufferPtr();
	wsabufs[0].len = dSize;

	wsabufs[1].buf = mRecvBuffer.GetBuffer();
	wsabufs[1].len = mRecvBuffer.GetFreeSize() - dSize;

	DWORD retval[2] = { 0, };

	/*WSABUF wsabuf;
	int dSize = mRecvBuffer.DirectEnqueueSize();

	wsabuf.buf = mRecvBuffer.GetRearBufferPtr();
	wsabuf.len = dSize;*/

	//DWORD retval = 0;

	DWORD flags = 0;

	int result = WSARecv(mSocket, wsabufs, 2, retval, &flags, NULL, NULL);

	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
			return;

		ErrorQuit(L"Receive Error to recv buffer", __FILEW__, __LINE__);

		return;
	}

	mRecvBuffer.MoveRear(retval[0] + retval[1]);
}

void Session::recvProc()
{
	while (1)
	{
		if (mRecvBuffer.GetUseSize() < sizeof(stHeader))
			break;

		stHeader header;
		mRecvBuffer.Peek((char*)&header, sizeof(stHeader));

		if (header.byCode != 0x89)
			ErrorQuit(L"Code is not matched", __FILEW__, __LINE__);

		if (mRecvBuffer.GetUseSize() < header.bySize + sizeof(stHeader))
			break;

		mRecvBuffer.MoveFront(sizeof(stHeader));

		readMessage(&header);
	}
}

void Session::readMessage(stHeader* header)
{
	CPacket packet;
	mRecvBuffer.Dequeue(packet.GetBufferPtr(), header->bySize);
	packet.MoveRear(header->bySize);

	switch (header->byType)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
	{
		CreateMyPlayer(&packet);
		break;
	}
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
	{
		CreateOtherPlayer(&packet);
		break;
	}
	case dfPACKET_SC_DELETE_CHARACTER:
	{
		DeletePlayer(&packet);
		break;
	}
	case dfPACKET_SC_MOVE_START:
	{
		MoveStartPlayer(&packet);
		break;
	}
	case dfPACKET_SC_MOVE_STOP:
	{
		MoveStopPlayer(&packet);
		break;
	}
	case dfPACKET_SC_ATTACK1:
	{
		AttackProc1(&packet);
		break;
	}
	case dfPACKET_SC_ATTACK2:
	{
		AttackProc2(&packet);
		break;
	}
	case dfPACKET_SC_ATTACK3:
	{
		AttackProc3(&packet);
		break;
	}
	case dfPACKET_SC_DAMAGE:
	{
		DamageProc(&packet);
		break;
	}
	case dfPACKET_SC_SYNC:
	{
		SyncProc(&packet);
		break;
	}
	default:
		break;
	}
}

void Session::ErrorQuit(const WCHAR* msg, const WCHAR* fileName, unsigned int lineNum)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	wprintf(L"%s : %d\n", fileName, lineNum);
	swprintf_s(errorMsg, _countof(errorMsg), L"%s code : %d\n", msg, err);
	MessageBox(gMainWindow, errorMsg, L"접속종료", MB_OK);

	exit(1);
}

void Session::ErrorDisplay(const WCHAR* msg)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	swprintf_s(errorMsg, _countof(errorMsg), L"%s code : %d\n", msg, err);
	MessageBox(gMainWindow, errorMsg, L"에러 발생", MB_OK);
}

void Session::CreateMyPlayer(CPacket* packet)
{
	PlayerObject* player = new PlayerObject;
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;
	BYTE HP;

	*packet >> ID >> direction >> x >> y >> HP;

	player->SetPosition(x, y);
	player->SetID(ID);
	player->SetHP(HP);
	player->SetDirection(direction);
	gPlayerObject = player;
	gObjectList.push_back(gPlayerObject);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Create Character ID: %d, X : %d, Y: %d\n",
		player->GetObectID(), player->GetCurX(), player->GetCurY());
}

void Session::CreateOtherPlayer(CPacket* packet)
{
	PlayerObject* player = new PlayerObject;
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;
	BYTE HP;

	*packet >> ID >> direction >> x >> y >> HP;

	player->SetPosition(x, y);
	player->SetID(ID);
	player->SetHP(HP);
	player->SetDirection(direction);
	player->SetEnemy();
	gObjectList.push_back(player);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Create Enemy ID: %d, X : %d, Y: %d\n",
		player->GetObectID(), player->GetCurX(), player->GetCurY());
}

void Session::DeletePlayer(CPacket* packet)
{
	DWORD ID;

	*packet >> ID;

	BaseObject* target = SearchObject(ID);

	if (target == nullptr)
		return;

	target->SetDestroy();

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Delete Character ID: %d, X : %d, Y: %d\n",
		target->GetObectID(), target->GetCurX(), target->GetCurY());
}

void Session::MoveStartPlayer(CPacket* packet)
{
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> ID >> direction >> x >> y;

	BaseObject* target = SearchObject(ID);

	if (target == nullptr)
		return;

	target->SetPosition(x, y);
	target->ActionInput(direction);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Move Character ID: %d, X : %d, Y: %d\n",
		target->GetObectID(), target->GetCurX(), target->GetCurY());
}

void Session::MoveStopPlayer(CPacket* packet)
{
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> ID >> direction >> x >> y;

	BaseObject* target = SearchObject(ID);

	if (target == nullptr)
		return;

	target->SetPosition(x, y);
	target->ActionInput(dfAction_STAND);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Stop Character ID: %d, X : %d, Y: %d\n",
		target->GetObectID(), target->GetCurX(), target->GetCurY());
}

void Session::AttackProc1(CPacket* packet)
{
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> ID >> direction >> x >> y;

	BaseObject* attacker = SearchObject(ID);

	if (attacker == nullptr)
		return;

	attacker->SetPosition(x, y);
	attacker->ActionInput(dfACTION_ATTACK1);
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Attack 1. ID: %d, X : %d, Y: %d\n",
		attacker->GetObectID(), attacker->GetCurX(), attacker->GetCurY());
}

void Session::AttackProc2(CPacket* packet)
{
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> ID >> direction >> x >> y;

	BaseObject* attacker = SearchObject(ID);

	if (attacker == nullptr)
		return;

	attacker->SetPosition(x, y);
	attacker->ActionInput(dfACTION_ATTACK2);
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Attack 2. ID: %d, X : %d, Y: %d\n",
		attacker->GetObectID(), attacker->GetCurX(), attacker->GetCurY());
}

void Session::AttackProc3(CPacket* packet)
{
	DWORD ID;
	BYTE direction;
	WORD x;
	WORD y;

	*packet >> ID >> direction >> x >> y;

	BaseObject* attacker = SearchObject(ID);

	if (attacker == nullptr)
		return;

	attacker->SetPosition(x, y);
	attacker->ActionInput(dfACTION_ATTACK3);
	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Attack 3. ID: %d, X : %d, Y: %d\n",
		attacker->GetObectID(), attacker->GetCurX(), attacker->GetCurY());
}

void Session::DamageProc(CPacket* packet)
{
	DWORD attackID;
	DWORD damageID;
	BYTE damageHP;

	*packet >> attackID >> damageID >> damageHP;

	PlayerObject* attacker = (PlayerObject*)SearchObject(attackID);
	PlayerObject* target = (PlayerObject*)SearchObject(damageID);

	if (target == nullptr)
		return;

	target->SetHP(damageHP);

	if (attacker == nullptr)
	{
		target->CreateEffectMySelf();
	}
	else
	{
		attacker->CreateEffect();
	}

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Get Damage. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(), target->GetCurX(), target->GetCurY());
}

void Session::SyncProc(CPacket* packet)
{
	DWORD targetID;
	WORD x;
	WORD y;

	*packet >> targetID >> x >> y;

	PlayerObject* target = (PlayerObject*)SearchObject(targetID);

	if (target == nullptr)
		return;

	target->SetPosition(x, y);

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"Sync. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(), target->GetCurX(), target->GetCurY());
}

BaseObject* Session::SearchObject(int id)
{
	//wprintf_s(L"서치 유저 시작\n");
	for (auto iter = gObjectList.begin(); iter != gObjectList.end(); ++iter)
	{
		//wprintf_s(L"서치할 유저 : %d, 현재 유저: %d\n", id, (*iter)->GetObectID());
		if ((*iter)->GetObectID() == id)
		{
			return *iter;
		}
	}

	return nullptr;
}
