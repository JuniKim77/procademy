#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"
#include <stdio.h>
#include "NetworkProcs.h"
#include <unordered_map>
#include "Session.h"
#include <vector>
#include "CPacket.h"
#include "User.h"

using namespace std;

SOCKET g_listenSocket;
DWORD g_SessionNo = 1;
DWORD g_RoomNo = 1;
unordered_map<DWORD, Session*> g_sessions;
unordered_map<DWORD, User*> g_users;
unordered_map<DWORD, Room*> g_rooms;

void CreateServer()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		exit(1);
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(dfNETWORK_PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	g_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_listenSocket == INVALID_SOCKET)
	{
		LogError(L"소켓 생성 에러", g_listenSocket);

		exit(1);
	}

	if (bind(g_listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		LogError(L"바인딩 에러", g_listenSocket);

		exit(1);
	}

	u_long on = 1;
	if (ioctlsocket(g_listenSocket, FIONBIO, &on) == SOCKET_ERROR)
	{
		LogError(L"논블락 소켓 전환 에러", g_listenSocket);

		exit(1);
	}

	if (listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		LogError(L"리스닝 에러", g_listenSocket);

		exit(1);
	}
}

void NetWorkProc()
{
	FD_SET rset;
	FD_SET wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	DWORD sessionKeyTable[FD_SETSIZE];

	FD_SET(g_listenSocket, &rset);

	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	int numSelected = select(0, &rset, NULL, NULL, &tval);

	if (numSelected == 1)
	{
		AcceptProc();
	}

	int count = 0;
	FD_ZERO(&rset);

	for (auto iter = g_sessions.begin(); iter != g_sessions.end();)
	{
		Session* session = iter->second;
		iter++; // 미리 증가

		sessionKeyTable[count] = session->mSessionNo;
		FD_SET(session->mSocket, &rset);

		if (session->mSendBuffer.GetUseSize() > 0)
		{
			FD_SET(session->mSocket, &wset);
		}

		count++;

		if (count == FD_SETSIZE)
		{
			SelectProc(sessionKeyTable, &rset, &wset);

			FD_ZERO(&rset);
			FD_ZERO(&wset);
			memset(sessionKeyTable, 0, sizeof(sessionKeyTable));
			count = 0;
		}
	}
	
	if (count > 0)
	{
		SelectProc(sessionKeyTable, &rset, &wset);
	}
}

void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset)
{
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;

	int numSelected = select(0, rset, wset, NULL, &tval);

	if (numSelected == SOCKET_ERROR)
	{
		LogError(L"Select 에러", g_listenSocket);

		exit(1);
	}

	for (int i = 0; i < FD_SETSIZE; ++i)
	{
		DWORD key = keyTable[i];
		if (g_sessions[key] == nullptr)
			continue;

		if (FD_ISSET(g_sessions[key]->mSocket, wset))
		{
			g_sessions[key]->writePacket();
		}

		if (FD_ISSET(g_sessions[key]->mSocket, rset))
		{
			g_sessions[key]->receivePacket();
		}

		// DisconnectProc

	}
}

void AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int len = sizeof(len);

	SOCKET client = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &len);

	if (client == INVALID_SOCKET)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			return;
		}
		LogError(L"Accept 에러", g_listenSocket);

		exit(1);
	}

	u_long on = 1;
	if (ioctlsocket(client, FIONBIO, &on) == SOCKET_ERROR) {
		// ???

		LogError(L"논블락 소켓 전환 에러", client);

		exit(1);
	}

	Session* session = new Session(client, ntohs(clientAddr.sin_port),
		clientAddr.sin_addr.S_un.S_addr);

	g_sessions[client] = session;
}

void LogError(const WCHAR* msg, SOCKET sock)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s : %d\n", msg, err);
	if (sock != INVALID_SOCKET)
	{
		closesocket(sock);
	}
}

BYTE makeCheckSum(CPacket* packet, WORD msgType)
{
	int size = packet->GetSize();
	BYTE* pBuf = (BYTE*)packet->GetBufferPtr();
	BYTE checkSum = 0;

	for (int i = 0; i < size; ++i)
	{
		checkSum += *pBuf;
		++pBuf;
	}

	return checkSum % 256;
}
