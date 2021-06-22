#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Protocol.h"
#include <stdio.h>
#include "NetworkProcs.h"
#include <unordered_map>
#include "Session.h"
#include <vector>

using namespace std;

SOCKET g_listenSocket;
DWORD g_NoUser = 0;
unordered_map<DWORD, Session*> g_sessions;

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
	FD_ZERO(&rset);

	FD_SET(g_listenSocket, &rset);

	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 100;

	int numSelected = select(0, &rset, NULL, NULL, &tval);

	if (numSelected == 1)
	{
		AcceptProc();
	}

	int count = 0;
	FD_ZERO(&rset);

	for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter)
	{
		FD_SET(iter->second->mSocket, &rset);
		count++;

		if (count == FD_SETSIZE)
		{
			// reset
			numSelected = select(0, &rset, NULL, NULL, &tval);

			for (int i = 0; i < numSelected;)
			{

			}

			FD_ZERO(&rset);
		}
	}
}

void AcceptProc()
{
	while (1)
	{
		SOCKADDR_IN clientAddr;
		int len = sizeof(len);

		SOCKET client = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &len);

		if (client == INVALID_SOCKET)
		{
			int err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK)
			{
				break;
			}
			LogError(L"Accept 에러", g_listenSocket);

			exit(1);
		}

		u_long on = 1;
		if (ioctlsocket(client, FIONBIO, &on) == SOCKET_ERROR) {
			// ???

			LogError(L"논블락 소켓 전환 에러", client);

			continue;
		}

		Session* session = new Session(client, ntohs(clientAddr.sin_port),
			clientAddr.sin_addr.S_un.S_addr, g_NoUser++);

		g_sessions[session->mIDNum] = session;
	}
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
