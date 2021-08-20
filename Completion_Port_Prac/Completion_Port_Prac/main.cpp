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

struct OverlappedBuffer
{
	WSAOVERLAPPED overlapped;
	bool type;
	RingBuffer queue;
};

struct Session
{
	OverlappedBuffer recv;
	OverlappedBuffer send;
	SOCKET socket;
	bool disconnect;
	u_short port;
	ULONG ip;

	Session(SOCKET _socket, ULONG _ip, u_short _port)
		: socket(_socket)
		, ip(_ip)
		, port(_port)
		, disconnect(false)
	{
		ZeroMemory(&recv.overlapped, sizeof(recv.overlapped));
		ZeroMemory(&send.overlapped, sizeof(send.overlapped));
		recv.type = true;
		send.type = false;
	}
};

unsigned int WINAPI workerThread(LPVOID arg);
unsigned int WINAPI acceptThread(LPVOID arg);
void DisconnectProc(Session* session);
void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv);

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

	g_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

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
	while (1)
	{
		DWORD transferredSize = 0;
		Session* completionKey = nullptr;
		WSAOVERLAPPED* pOverlapped = nullptr;
		Session* session = nullptr;

		BOOL retval = GetQueuedCompletionStatus(g_hcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);

		if (retval == FALSE)
		{
			errorLog(L"GetQueuedCompletionStatus");
		}

		if (pOverlapped == nullptr)  // I/O Fail
		{
			continue;
		}

		if (transferredSize == 0) // normal close
		{
			if (*(bool*)(pOverlapped + 1) == true) // recv
			{
				session = (Session*)pOverlapped;
			}
			else // send
			{
				session = (Session*)((OverlappedBuffer*)pOverlapped - 1);
			}

			WCHAR IP[16] = { 0, };

			InetNtop(AF_INET, &session->ip, IP, 16);

			if (InterlockedExchange8((char*)(&session->disconnect), true) == true)
			{
				wprintf_s(L"Disconnect [IP: %s] [Port: %u]\n", IP, ntohs(session->port));
				DisconnectProc(session);
				continue;
			}
			else
			{
				PostQueuedCompletionStatus(g_hcp, 0, (ULONG_PTR)session, pOverlapped);

				continue;
			}
		}

		session = (Session*)completionKey;

		if (pOverlapped == &session->recv.overlapped) // Recv
		{
			char buf[3000 + 2];

			session->recv.queue.MoveRear(transferredSize);

			session->recv.queue.Dequeue(buf, transferredSize);

			buf[transferredSize] = '\0';
			buf[transferredSize + 1] = '\0';

			session->recv.queue.MoveFront(transferredSize);

			session->send.queue.Enqueue(buf, transferredSize);

			WSABUF buffers[2];
			DWORD flags = 0;

			SetWSABuf(buffers, session, true);

			WSABUF bufferSend[2];

			SetWSABuf(bufferSend, session, false);
			
			printf("Receive Data: %s\n", buf);


			WSASend(session->socket, bufferSend, 2, nullptr, 0, &session->send.overlapped, nullptr);
			WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recv.overlapped, nullptr);
		}
		else // send
		{
			char buf[3000 + 2];

			session->send.queue.Dequeue(buf, transferredSize);

			buf[transferredSize] = '\0';
			buf[transferredSize + 1] = '\0';

			session->send.queue.MoveFront(transferredSize);

			printf("Send Data: %s\n", buf);
		}
	}
	return 0;
}

unsigned int __stdcall acceptThread(LPVOID arg)
{
	while (1)
	{
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(g_listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			errorLog(L"Socket Accept");
			continue;
		}

		// 함수로 뺄 것
		Session* session = new Session(client_sock, clientaddr.sin_addr.S_un.S_addr, clientaddr.sin_port);

		WCHAR IP[16] = { 0, };
		InetNtop(AF_INET, &clientaddr.sin_addr, IP, 16);

		wprintf_s(L"[TCP] Connect Session [IP: %s][Port: %d]\n", IP, ntohs(clientaddr.sin_port));

		// 소켓과 입출력 완료 포트 연결
		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, g_hcp,
			(ULONG_PTR)session, 0);

		g_sessionList.push_back(session);

		if (hResult == NULL)
		{
			errorLog(L"CreateIoCompletionPort");
			return -1;
		}

		DWORD flags = 0;
		WSABUF buffers[2];

		SetWSABuf(buffers, session, true);

		int retval = WSARecv(client_sock, buffers, 2, nullptr,
			&flags, &(session->recv.overlapped), NULL);

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

void DisconnectProc(Session* session)
{
	g_sessionList.remove(session);
	closesocket(session->socket);
	delete session;
}

void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
{
	if (isRecv)
	{
		int dSize = session->recv.queue.DirectEnqueueSize();

		bufs[0].buf = session->recv.queue.GetRearBufferPtr();
		bufs[0].len = dSize;
		bufs[1].buf = session->recv.queue.GetBuffer();
		bufs[1].len = session->recv.queue.GetFreeSize() - dSize;
	}
	else
	{
		int dSize = session->send.queue.DirectDequeueSize();

		bufs[0].buf = session->send.queue.GetFrontBufferPtr();
		bufs[0].len = dSize;
		bufs[1].buf = session->send.queue.GetBuffer();
		bufs[1].len = session->send.queue.GetUseSize() - dSize;
	}	
}

void errorLog(const WCHAR* s)
{
	int err = WSAGetLastError();

	wprintf_s(L"%s, Error Code: %d\n", s, err);
}
