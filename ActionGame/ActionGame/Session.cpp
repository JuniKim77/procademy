#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include "Session.h"
#include "stdio.h"
#include "PacketDefine.h"
#include "myList.h"
#include "PlayerObject.h"
#include "EffectObject.h"
#include "ActionDefine.h"

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
		ErrorQuit(L"소켓 생성 에러", __FILEW__, __LINE__);

	/*WCHAR ServerIP[16];
	wprintf_s(L"서버 IP: ");
	_getws_s(ServerIP);*/

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	//InetPton(AF_INET, ServerIP, &addr.sin_addr);
	InetPton(AF_INET, L"127.0.0.1", &addr.sin_addr);

	int asyncselectRetval = WSAAsyncSelect(mSocket, hWnd, WM_SOCKET,
		FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);

	if (asyncselectRetval == SOCKET_ERROR) {
		ErrorDisplay(L"소켓 모델 전환 에러");
		return false;
	}

	int connectRetval = connect(mSocket, (SOCKADDR*)&addr, sizeof(addr));
	if (connectRetval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			ErrorDisplay(L"연결 에러");
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
		ErrorQuit(L"Send Error: 서버 연결 장애", __FILEW__, __LINE__);
}

void Session::writeProc()
{
	char buffer[10000];
	int peekSize = mSendBuffer.Peek(buffer, 10000);

	if (peekSize == 0)
		return;

	int sendSize = send(mSocket, buffer, peekSize, 0);

	if (sendSize == SOCKET_ERROR)
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
	char buffer[10000];
	int retval = recv(mSocket, buffer, mRecvBuffer.GetFreeSize(), 0);

	if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
			return;

		ErrorQuit(L"Receive Error: 서버 연결 장애", __FILEW__, __LINE__);
	}

	mRecvBuffer.Enqueue(buffer, retval);
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
			ErrorQuit(L"서버 이상", __FILEW__, __LINE__);

		if (mRecvBuffer.GetUseSize() < header.bySize + sizeof(stHeader))
			break;

		mRecvBuffer.MoveFront(sizeof(stHeader));

		readMessage(&header);
	}
}

void Session::readMessage(stHeader* header)
{
	switch (header->byType)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
	{
		scCreateMyCharacter packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		CreateMyPlayer((char*)&packet);
		break;
	}
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
	{
		scCreateOtherCharacter packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		CreateOtherPlayer((char*)&packet);
		break;
	}
	case dfPACKET_SC_DELETE_CHARACTER:
	{
		scDeleteCharacter packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		DeletePlayer((char*)&packet);
		break;
	}
	case dfPACKET_SC_MOVE_START:
	{
		scMoveStart packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		MoveStartPlayer((char*)&packet);
		break;
	}
	case dfPACKET_SC_MOVE_STOP:
	{
		scMoveStop packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		MoveStopPlayer((char*)&packet);
		break;
	}
	case dfPACKET_SC_ATTACK1:
	{
		scAttack1 packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		AttackProc1((char*)&packet);
		break;
	}
	case dfPACKET_SC_ATTACK2:
	{
		scAttack2 packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		AttackProc2((char*)&packet);
		break;
	}
	case dfPACKET_SC_ATTACK3:
	{
		scAttack3 packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		AttackProc3((char*)&packet);
		break;
	}
	case dfPACKET_SC_DAMAGE:
	{
		scDamage packet;
		mRecvBuffer.Dequeue((char*)&packet, sizeof(packet));

		DamageProc((char*)&packet);
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
	MessageBox(gMainWindow, errorMsg, L"접속종료", MB_OK);

	exit(1);
}

void Session::ErrorDisplay(const WCHAR* msg)
{
	int err = WSAGetLastError();
	WCHAR errorMsg[100];
	swprintf_s(errorMsg, _countof(errorMsg), L"%s code : %d", msg, err);
	MessageBox(gMainWindow, errorMsg, L"에러 발생", MB_OK);
}

void Session::CreateMyPlayer(const char* msg)
{
	PlayerObject* player = new PlayerObject;
	scCreateMyCharacter* packet = (scCreateMyCharacter*)msg;

	player->SetPosition(packet->X, packet->Y);
	player->SetID(packet->ID);
	player->SetHP(packet->HP);
	player->SetDirection(packet->Direction);
	gPlayerObject = player;
	gObjectList.push_back(gPlayerObject);

	wprintf_s(L"내 캐릭터 생성 ID: %d, X : %d, Y: %d\n",
		player->GetObectID(), player->GetCurX(), player->GetCurY());
}

void Session::CreateOtherPlayer(const char* msg)
{
	PlayerObject* player = new PlayerObject;
	scCreateOtherCharacter* packet = (scCreateOtherCharacter*)msg;

	player->SetPosition(packet->X, packet->Y);
	player->SetID(packet->ID);
	player->SetHP(packet->HP);
	player->SetDirection(packet->Direction);
	player->SetEnemy();
	gObjectList.push_back(player);

	wprintf_s(L"적 캐릭터 생성 ID: %d, X : %d, Y: %d\n",
		player->GetObectID(), player->GetCurX(), player->GetCurY());
}

void Session::DeletePlayer(const char* msg)
{
	scDeleteCharacter* packet = (scDeleteCharacter*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetDestroy();
	wprintf_s(L"캐릭터 삭제 ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::MoveStartPlayer(const char* msg)
{
	scMoveStart* packet = (scMoveStart*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetPosition(packet->X, packet->Y);
	target->ActionInput(packet->Direction);
	wprintf_s(L"캐릭터 이동 시작. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::MoveStopPlayer(const char* msg)
{
	scMoveStart* packet = (scMoveStart*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetPosition(packet->X, packet->Y);
	target->ActionInput(dfAction_STAND);
	wprintf_s(L"캐릭터 정지. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc1(const char* msg)
{
	scAttack1* packet = (scAttack1*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetPosition(packet->X, packet->Y);
	target->ActionInput(dfACTION_ATTACK1);
	wprintf_s(L"캐릭터 공격 1. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc2(const char* msg)
{
	scAttack2* packet = (scAttack2*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetPosition(packet->X, packet->Y);
	target->ActionInput(dfACTION_ATTACK2);
	wprintf_s(L"캐릭터 공격 2. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::AttackProc3(const char* msg)
{
	scAttack3* packet = (scAttack3*)msg;

	BaseObject* target = SearchObject(packet->ID);

	if (target == nullptr)
		return;

	target->SetPosition(packet->X, packet->Y);
	target->ActionInput(dfACTION_ATTACK3);
	wprintf_s(L"캐릭터 공격 2. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

void Session::DamageProc(const char* msg)
{
	scDamage* packet = (scDamage*)msg;

	PlayerObject* attacker = (PlayerObject*)SearchObject(packet->AttackID);
	PlayerObject* target = (PlayerObject*)SearchObject(packet->DamageID);

	if (attacker == nullptr || target == nullptr)
		return;

	target->SetHP(packet->DamageHP);
	attacker->CreateEffect();

	wprintf_s(L"캐릭터 공격 받음. ID: %d, X : %d, Y: %d\n",
		target->GetObectID(),
		target->GetCurX(),
		target->GetCurY());
}

BaseObject* Session::SearchObject(int id)
{
	wprintf_s(L"서치 유저 시작\n");
	for (auto iter = gObjectList.begin(); iter != gObjectList.end(); ++iter)
	{
		wprintf_s(L"서치할 유저 : %d, 현재 유저: %d\n", id, (*iter)->GetObectID());
		if ((*iter)->GetObectID() == id)
		{
			return *iter;
		}
	}

	return nullptr;
}
