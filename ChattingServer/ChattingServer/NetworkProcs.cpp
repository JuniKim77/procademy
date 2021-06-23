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
	auto fdIter = g_sessions.begin();

	for (auto iter = g_sessions.begin(); iter != g_sessions.end();)
	{
		FD_SET(iter->second->mSocket, &rset);
		count++;

		if (count == FD_SETSIZE)
		{
			SelectProc(fdIter, &rset);

			FD_ZERO(&rset);
			fdIter = ++iter;
		}
		else
		{
			++iter;
		}
	}
	
	SelectProc(fdIter, &rset);

	//DestroySessionProc();
}

void SelectProc(std::unordered_map<DWORD, Session*>::iterator iter, FD_SET* rset)
{
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 100;

	int numSelected = select(0, rset, NULL, NULL, &tval);

	if (numSelected == SOCKET_ERROR)
	{
		LogError(L"Select 에러", g_listenSocket);

		exit(1);
	}

	for (int i = 0; i < numSelected;)
	{
		if (FD_ISSET(iter->second->mSocket, rset))
		{
			iter->second->receivePacket();
			
			i++;
		}
		iter++;
	}
}

void DestroySessionProc()
{
	for (auto iter = g_sessions.begin(); iter != g_sessions.end();)
	{
		if (iter->second->mbAlive == false)
		{
			// 삭제 코드
		}
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
