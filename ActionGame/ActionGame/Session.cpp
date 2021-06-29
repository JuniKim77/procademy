#pragma comment(lib, "ws2_32")

#define DEBUG
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

extern HWND gMainWindow;
extern myList<BaseObject*> gObjectList;
extern BaseObject* gPlayerObject;

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
		ErrorQuit(L"���� ���� ����", __FILEW__, __LINE__);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);

#ifdef DEBUG
	InetPton(AF_INET, L"127.0.0.1", &addr.sin_addr);
#else
	WCHAR ServerIP[16];
	wprintf_s(L"���� IP: ");
	_getws_s(ServerIP);
	InetPton(AF_INET, ServerIP, &addr.sin_addr);
#endif // DEBUG

	int asyncselectRetval = WSAAsyncSelect(mSocket, hWnd, WM_SOCKET,
		FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);

	if (asyncselectRetval == SOCKET_ERROR) {
		ErrorDisplay(L"���� �� ��ȯ ����");
		return false;
	}

	int connectRetval = connect(mSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (connectRetval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			ErrorDisplay(L"���� ����");
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
		ErrorQuit(L"Send Error: ���� ���� ���", __FILEW__, __LINE__);
}

void Session::writeProc()
{
	int useSize = mSendBuffer.GetUseSize();
	int directDequeueSize = mSendBuffer.DirectDequeueSize();
	int size = 0;
	int remain = 0;
	
	if (useSize > directDequeueSize)
	{
		size = directDequeueSize;
		remain = useSize - size;
	}
	else
	{
		size = useSize;
	}

	sendTCP(size);
	sendTCP(remain);
}

void Session::sendTCP(int size)
{
	if (size == 0)
		return;

	int sendSize = send(mSocket, mSendBuffer.GetFrontBufferPtr(), size, 0);

	if (sendSize == SOCKET_ERROR || sendSize < size)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}

		ErrorQuit(L"Send Error", __FILEW__, __LINE__);
	}

	mSendBuffer.MoveFront(sendSize);
}

void Session::ReceivePacket()
{
	int freeSize = mRecvBuffer.GetFreeSize();
	int directEnqueueSize = mRecvBuffer.DirectEnqueueSize();
	int size = 0;
	int remain = 0;

	if (freeSize > directEnqueueSize)
	{
		size = directEnqueueSize;
		remain = freeSize - directEnqueueSize;
	}
	else
	{
		size = freeSize;
	}

	receiveTCP(size);
	receiveTCP(remain);
}

void Session::receiveTCP(int size)
{
	if (size == 0)
		return;

	int retval = recv(mSocket, mRecvBuffer.GetRearBufferPtr(), size, 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;


		// ���� �ʿ�?
		ErrorQuit(L"Receive Error: ���� ���� ���", __FILEW__, __LINE__);
	}

	mRecvBuffer.MoveRear(retval);
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
			ErrorQuit(L"���� �̻�", __FILEW__, __LINE__);

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
	swprintf_s(errorMsg, 100, L"%s code : %d", msg, err);
	MessageBox(gMainWindow, errorMsg, L"��������", MB_OK);

	exit(1);
}

void Session::ErrorDisplay(const WCHAR* msg)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	swprintf_s(errorMsg, _countof(errorMsg), L"%s code : %d", msg, err);
	MessageBox(gMainWindow, errorMsg, L"���� �߻�", MB_OK);
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

	wprintf_s(L"�� ĳ���� ���� ID: %d, X : %d, Y: %d\n",
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

	wprintf_s(L"�� ĳ���� ���� ID: %d, X : %d, Y: %d\n",
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
	wprintf_s(L"ĳ���� ���� ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
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
	wprintf_s(L"ĳ���� �̵� ����. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
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
	wprintf_s(L"ĳ���� ����. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc1(CPacket* packet)
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
	target->ActionInput(dfACTION_ATTACK1);
	wprintf_s(L"ĳ���� ���� 1. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc2(CPacket* packet)
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
	target->ActionInput(dfACTION_ATTACK2);
	wprintf_s(L"ĳ���� ���� 2. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc3(CPacket* packet)
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
	target->ActionInput(dfACTION_ATTACK3);
	wprintf_s(L"ĳ���� ���� 2. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::DamageProc(CPacket* packet)
{
	DWORD attackID;
	DWORD damageID;
	BYTE damageHP;

	*packet >> attackID >> damageID >> damageHP;

	PlayerObject* attacker = (PlayerObject*)SearchObject(attackID);
	PlayerObject* target = (PlayerObject*)SearchObject(damageID);

	if (attacker == nullptr || target == nullptr)
		return;

	target->SetHP(damageHP);
	attacker->CreateEffect();

	wprintf_s(L"ĳ���� ���� ����. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
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

	wprintf_s(L"ĳ���� ����ȭ. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

BaseObject* Session::SearchObject(int id)
{
	//wprintf_s(L"��ġ ���� ����\n");
	for (auto iter = gObjectList.begin(); iter != gObjectList.end(); ++iter)
	{
		//wprintf_s(L"��ġ�� ���� : %d, ���� ����: %d\n", id, (*iter)->GetObectID());
		if ((*iter)->GetObectID() == id)
		{
			return *iter;
		}
	}

	return nullptr;
}
