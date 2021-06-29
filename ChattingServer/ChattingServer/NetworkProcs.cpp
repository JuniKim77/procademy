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
#include "Container.h"
#include "PacketCreater.h"
#include "Content.h"

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
	ZeroMemory(&addr, sizeof(addr));
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

	wprintf_s(L"Server Open...\n");
}

void NetWorkProc()
{
	FD_SET rset;
	FD_SET wset;
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	DWORD sessionKeyTable[FD_SETSIZE] = { 0, };

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

		if (session == nullptr)
			continue;

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
	/*Session* session = FindSession(1);
	if (session != nullptr)
	{
		sessionKeyTable[0] = session->mSessionNo;
		FD_SET(session->mSocket, &rset);

		if (session->mSendBuffer.GetUseSize() > 0)
		{
			FD_SET(session->mSocket, &wset);
		}

		count++;
	}*/
	
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
		Session* session = FindSession(keyTable[i]);

		if (session == nullptr)
			continue;

		if (FD_ISSET(session->mSocket, wset))
		{
			session->writePacket();
		}

		if (FD_ISSET(session->mSocket, rset))
		{
			session->receivePacket();
		}

		// DisconnectProc
		if (session->mbAlive == false)
		{
			DisconnectProc(session->mSessionNo);
		}
	}
}

void DisconnectProc(DWORD sessionKey)
{
	User* user = FindUser(sessionKey);

	if (user->mRoomNo != 0)
	{
		ReqRoomLeave(sessionKey);
	}

	DeleteSessionData(sessionKey);
	DeleteUserData(sessionKey);
}

void AcceptProc()
{
	SOCKADDR_IN clientAddr;
	int len = sizeof(clientAddr);

	SOCKET client = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &len);

	WCHAR temp[16] = { 0, };
	InetNtop(AF_INET, &clientAddr.sin_addr, temp, 16);

	wprintf_s(L"Accept: IP - %s, 포트 - %d [UserNo: %d]\n",
		temp, ntohs(clientAddr.sin_port), g_SessionNo);

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
		clientAddr.sin_addr.S_un.S_addr, g_SessionNo);

	// 껍데기 유저
	User* user = new User(g_SessionNo);

	InsertSessionData(g_SessionNo, session);
	InsertUserData(g_SessionNo, user);

	g_SessionNo++;
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