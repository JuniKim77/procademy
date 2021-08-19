#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "RingBuffer.h"
#include <process.h>
#include <conio.h>
#include <list>
#include <WS2tcpip.h>

using namespace std;

#define dfTHREAD_NUM (4)

struct Session
{
	SOCKET socket;
	WSAOVERLAPPED sendOverlapped;
	bool sendType;
	RingBuffer sendQueue;
	WSAOVERLAPPED recvOverlapped;
	bool recvType;
	RingBuffer recvQueue;
	bool disconnect;
	u_short port;
	ULONG ip;
};

unsigned int WINAPI workerThread(LPVOID arg);
unsigned int WINAPI acceptThread(LPVOID arg);

void errorLog(const WCHAR* s);

SOCKET g_listen_sock;
HANDLE g_hcp;

list<Session*> g_sessionList;

int main()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		errorLog(L"WSAStartup");
		return -1;
	}

	g_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET)
	{
		errorLog(L"Socket Create");
		return -1;
	}

	SOCKADDR_IN socketAddr;
	ZeroMemory(&socketAddr, sizeof(socketAddr));
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(9000);
	socketAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int retval = bind(g_listen_sock, (SOCKADDR*)&socketAddr, sizeof(socketAddr));
	if (retval == SOCKET_ERROR)
	{
		errorLog(L"Socket Bind");
		closesocket(g_listen_sock);
		return -1;
	}

	retval = listen(g_listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		errorLog(L"Socket Listen");
		closesocket(g_listen_sock);
		return -1;
	}

	HANDLE g_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (g_hcp == NULL)
	{
		errorLog(L"CreateIoCompletionPort");
		closesocket(g_listen_sock);
		return -1;
	}

	HANDLE hThreads[dfTHREAD_NUM];

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, acceptThread, nullptr, 0, nullptr);

	for (int i = 1; i < dfTHREAD_NUM; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, nullptr, 0, nullptr);
	}

	while (1)
	{
		WCHAR controlKey = _getwch();
		rewind(stdin);

		if (controlKey == L'Q')
		{
			// 종료 처리
			wprintf_s(L"Exit\n");

			break;
		}
	}

	DWORD waitResult = WaitForMultipleObjects(dfTHREAD_NUM, hThreads, TRUE, INFINITE);

	switch (waitResult)
	{
	case WAIT_FAILED:
		wprintf_s(L"Main Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		wprintf_s(L"Main Thread Timeout Error\n");
		break;
	case WAIT_OBJECT_0:
		wprintf_s(L"None Error\n");
		break;
	default:
		break;
	}

	//------------------------------------------------
	// 디버깅용 코드  스레드 정상종료 확인.
	//------------------------------------------------
	DWORD ExitCode;

	wprintf_s(L"\n\n--- THREAD CHECK LOG -----------------------------\n\n");

	GetExitCodeThread(hThreads[0], &ExitCode);
	if (ExitCode != 0)
		wprintf_s(L"error - Accept Thread not exit\n");

	for (int i = 1; i < dfTHREAD_NUM; ++i)
	{
		GetExitCodeThread(hThreads[i], &ExitCode);
		if (ExitCode != 0)
			wprintf_s(L"error - Worker Thread not exit\n");
	}

	return 0;
}

unsigned int __stdcall workerThread(LPVOID arg)
{
	HANDLE hcp = g_hcp;
	
	while (1)
	{
		DWORD trandferredSize = 0;
		SOCKET ptr = 0;
		LPOVERLAPPED* pOverlapped;
		BOOL retval = GetQueuedCompletionStatus(hcp, &trandferredSize, (PULONG_PTR)&ptr, (LPOVERLAPPED*)pOverlapped, INFINITE);

		Session* session = (Session*)ptr;




	}
	return 0;
}

unsigned int __stdcall acceptThread(LPVOID arg)
{
	HANDLE hcp = g_hcp;
	SOCKET listen_sock = g_listen_sock;

	while (1)
	{
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			errorLog(L"Socket Accept");
			continue;
		}

		// 함수로 뺄 것
		Session* session = new Session;
		session->disconnect = false;
		session->port = clientaddr.sin_port;
		session->ip = clientaddr.sin_addr.S_un.S_addr;
		session->recvType = true;
		session->sendType = false;

		WCHAR IP[16] = { 0, };
		InetNtop(AF_INET, &clientaddr.sin_addr, IP, 16);

		wprintf_s(L"[TCP] Connect Session [IP: %s][Port: %d]\n", IP, ntohs(clientaddr.sin_port));

		// 소켓과 입출력 완료 포트 연결
		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, hcp,
			(DWORD)session, 0);

		g_sessionList.push_back(session);

		if (hResult == NULL)
		{
			errorLog(L"CreateIoCompletionPort");
			return -1;
		}

		DWORD flags = 0;
		WSABUF buffers[2];

		buffers[0].buf = session->recvQueue.GetRearBufferPtr();
		buffers[0].len = session->recvQueue.DirectEnqueueSize();
		buffers[1].buf = session->recvQueue.GetBuffer();
		buffers[1].len = 0;

		int retval = WSARecv(client_sock, buffers, 2, nullptr,
			&flags, &(session->recvOverlapped), NULL);

		if (retval == SOCKET_ERROR) 
		{
			if (WSAGetLastError() != ERROR_IO_PENDING) 
			{
				errorLog(L"WSARecv");
				return -1;
			}
			continue;
		}
	}

	return 0;
}

void errorLog(const WCHAR* s)
{
	int err = WSAGetLastError();

	wprintf_s(L"%s, Error Code: %d\n", s, err);
}
