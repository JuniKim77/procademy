#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "RingBuffer.h"
#include <process.h>
#include <conio.h>
#include <list>
#include <WS2tcpip.h>
#include "ObjectPool.h"
#include "CPacket.h"

using namespace std;

#define dfTHREAD_NUM (4)
#define dfSERVER_PORT (6000)
#define dfMESSAGE_SIZE (10)

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
	char ioCount;
	bool isSending;
	u_short port;
	ULONG ip;
	SRWLOCK lockObj;

	Session()
		: ioCount(0)
		, isSending(false)
	{
		ZeroMemory(&recv.overlapped, sizeof(recv.overlapped));
		ZeroMemory(&send.overlapped, sizeof(send.overlapped));
		recv.type = true;
		send.type = false;
		InitializeSRWLock(&lockObj);
	}

	Session(SOCKET _socket, ULONG _ip, u_short _port)
		: socket(_socket)
		, ip(_ip)
		, port(_port)
		, ioCount(0)
		, isSending(false)
	{
		ZeroMemory(&recv.overlapped, sizeof(recv.overlapped));
		ZeroMemory(&send.overlapped, sizeof(send.overlapped));
		recv.type = true;
		send.type = false;
		InitializeSRWLock(&lockObj);
	}
};

struct Monitor
{
	unsigned short acceptCount;
	unsigned short disconnectCount;
	unsigned int sendTPS;
	unsigned int recvTPS;
	SRWLOCK lock;
};

unsigned int WINAPI workerThread(LPVOID arg);
unsigned int WINAPI acceptThread(LPVOID arg);
void DisconnectProc(Session* session);
bool DecrementProc(Session* session);
void MonitorProc();
void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv);
bool Initialize();
bool SendPost(Session* session);
bool RecvPost(Session* session);

void errorLog(const WCHAR* s);

SOCKET g_listen_sock;
HANDLE g_hcp;

list<Session*> g_sessionList;
SRWLOCK g_sessionListLock;
procademy::ObjectPool<Session> g_SessionPool(100);
Monitor g_monitor;

int main()
{
	if (!Initialize())
	{
		return -1;

	}

	HANDLE hThreads[dfTHREAD_NUM];

	hThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, acceptThread, nullptr, 0, nullptr);

	for (int i = 1; i < dfTHREAD_NUM; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, nullptr, 0, nullptr);
	}

	HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

	while (1)
	{
		DWORD retval = WaitForSingleObject(dummyEvent, 1000);

		if (retval == WAIT_TIMEOUT)
		{
			MonitorProc();

			if (GetAsyncKeyState(VK_TAB) & 0x8001)
			{
				// 종료 처리
				wprintf_s(L"Exit\n");

				PostQueuedCompletionStatus(g_hcp, 0, 0, 0);

				closesocket(g_listen_sock);

				break;
			}
		}
		else
		{
			errorLog(L"Something Error");
			PostQueuedCompletionStatus(g_hcp, 0, 0, 0);

			closesocket(g_listen_sock);
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

		/*if (retval == FALSE)
		{
			errorLog(L"GetQueuedCompletionStatus");
		}*/

		if (transferredSize == 0 && (PULONG_PTR)completionKey == 0 && pOverlapped == nullptr)
		{
			PostQueuedCompletionStatus(g_hcp, 0, 0, 0);

			break;
		}

		if (pOverlapped == nullptr)  // I/O Fail
		{
			continue;
		}

		session = (Session*)completionKey;

		if (transferredSize == 0) // normal close
		{
			DecrementProc(session);

			continue;
		}

		if (pOverlapped == &session->recv.overlapped) // Recv
		{
			CPacket packet;
			// Dequeue Proc
			session->recv.queue.MoveRear(transferredSize);

			int messageCount = transferredSize / dfMESSAGE_SIZE;
			int messageByte = messageCount * dfMESSAGE_SIZE;

			session->recv.queue.Dequeue(packet.GetBufferPtr(), messageByte);

			// Packet Proc
			session->send.queue.Lock(false);
			session->send.queue.Enqueue(packet.GetBufferPtr(), messageByte);
			session->send.queue.Unlock(false);

			AcquireSRWLockExclusive(&g_monitor.lock);
			g_monitor.recvTPS += messageCount;
			ReleaseSRWLockExclusive(&g_monitor.lock);

			AcquireSRWLockExclusive(&session->lockObj);
			// Send Post
			SendPost(session);

			// Recv Post
			RecvPost(session);
			ReleaseSRWLockExclusive(&session->lockObj);
		}
		else // send
		{
			int messageCount = transferredSize / dfMESSAGE_SIZE;
			int messageByte = messageCount * dfMESSAGE_SIZE;

			AcquireSRWLockExclusive(&g_monitor.lock);
			g_monitor.sendTPS += messageCount;
			ReleaseSRWLockExclusive(&g_monitor.lock);

			AcquireSRWLockExclusive(&session->lockObj);

			session->isSending = false;

			if (DecrementProc(session))
				continue;

			session->send.queue.MoveFront(messageByte);

			if (session->send.queue.GetUseSize() > 0)
			{
				SendPost(session);
			}

			ReleaseSRWLockExclusive(&session->lockObj);
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
			int err = WSAGetLastError();

			if (err == WSAENOTSOCK)
			{
				wprintf_s(L"Listen Socket Close\n");

				break;
			}

			wprintf_s(L"Socket Accept [Error: %d]\n", err);
			continue;
		}

		AcquireSRWLockExclusive(&g_monitor.lock);
		g_monitor.acceptCount++;
		ReleaseSRWLockExclusive(&g_monitor.lock);

		// 함수로 뺄 것
		g_SessionPool.Lock(false);
		Session* session = g_SessionPool.Alloc();
		g_SessionPool.Unlock(false);

		session->socket = client_sock;
		session->ip = clientaddr.sin_addr.S_un.S_addr;
		session->port = clientaddr.sin_port;

		ZeroMemory(&session->recv.overlapped, sizeof(session->recv.overlapped));
		ZeroMemory(&session->send.overlapped, sizeof(session->send.overlapped));
		InitializeSRWLock(&session->lockObj);

		// Zero Copy?

		/*WCHAR IP[16] = { 0, };
		InetNtop(AF_INET, &clientaddr.sin_addr, IP, 16);

		wprintf_s(L"[TCP] Connect Session [IP: %s][Port: %d]\n", IP, ntohs(clientaddr.sin_port));*/

		// 소켓과 입출력 완료 포트 연결
		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, g_hcp,
			(ULONG_PTR)session, 0);

		AcquireSRWLockExclusive(&g_sessionListLock);
		g_sessionList.push_back(session);
		ReleaseSRWLockExclusive(&g_sessionListLock);

		if (hResult == NULL)
		{
			errorLog(L"CreateIoCompletionPort");
			return -1;
		}

		session->ioCount = 1;
		RecvPost(session);
	}

	return 0;
}

void DisconnectProc(Session* session)
{
	AcquireSRWLockExclusive(&g_sessionListLock);
	g_sessionList.remove(session);
	ReleaseSRWLockExclusive(&g_sessionListLock);

	closesocket(session->socket);

	AcquireSRWLockExclusive(&g_monitor.lock);
	g_monitor.disconnectCount++;
	ReleaseSRWLockExclusive(&g_monitor.lock);

	//delete session;
	g_SessionPool.Lock(false);
	g_SessionPool.Free(session);
	g_SessionPool.Unlock(false);
}

bool DecrementProc(Session* session)
{
	session->ioCount--;

	if (session->ioCount == 0)
	{
		/*WCHAR IP[16] = { 0, };

		InetNtop(AF_INET, &session->ip, IP, 16);
		wprintf_s(L"Disconnect [IP: %s] [Port: %u]\n", IP, ntohs(session->port));*/
		DisconnectProc(session);

		return true;
	}

	return false;
}

void MonitorProc()
{
	AcquireSRWLockExclusive(&g_monitor.lock);

	wprintf_s(L"=======================================\n");
	wprintf_s(L"[Total Accept Count: %u]\n", g_monitor.acceptCount);
	wprintf_s(L"[Total Diconnect Count: %u]\n", g_monitor.disconnectCount);
	wprintf_s(L"[Live Session Count: %u]\n", g_monitor.acceptCount - g_monitor.disconnectCount);
	wprintf_s(L"=======================================\n");
	wprintf_s(L"[Send TPS: %u]\n", g_monitor.sendTPS);
	wprintf_s(L"[Recv TPS: %u]\n", g_monitor.recvTPS);
	wprintf_s(L"=======================================\n");
	g_monitor.sendTPS = 0;
	g_monitor.recvTPS = 0;

	ReleaseSRWLockExclusive(&g_monitor.lock);
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

bool Initialize()
{
	ZeroMemory(&g_monitor, sizeof(g_monitor));
	InitializeSRWLock(&g_monitor.lock);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		errorLog(L"WSAStartup");
		return false;
	}

	g_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET)
	{
		errorLog(L"Socket Create");
		return false;
	}

	SOCKADDR_IN socketAddr;
	ZeroMemory(&socketAddr, sizeof(socketAddr));
	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(dfSERVER_PORT);
	socketAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int retval = bind(g_listen_sock, (SOCKADDR*)&socketAddr, sizeof(socketAddr));
	if (retval == SOCKET_ERROR)
	{
		errorLog(L"Socket Bind");
		closesocket(g_listen_sock);
		return false;
	}

	retval = listen(g_listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		errorLog(L"Socket Listen");
		closesocket(g_listen_sock);
		return false;
	}

	g_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

	if (g_hcp == NULL)
	{
		errorLog(L"CreateIoCompletionPort");
		closesocket(g_listen_sock);
		return false;
	}

	InitializeSRWLock(&g_sessionListLock);

	return true;
}

bool SendPost(Session* session)
{
	WSABUF buffers[2];

	if (session->isSending)
	{
		return true;
	}

	session->isSending = true;

	SetWSABuf(buffers, session, false);

	session->ioCount++;
	int sendRet = WSASend(session->socket, buffers, 2, nullptr, 0, &session->send.overlapped, nullptr);

	if (sendRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		//wprintf_s(L"WSASend ERROR, Error Code: %d\n", err);

		if (DecrementProc(session))
			return false;
	}

	return true;
}

bool RecvPost(Session* session)
{
	WSABUF buffers[2];
	DWORD flags = 0;

	SetWSABuf(buffers, session, true);

	int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recv.overlapped, nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		//wprintf_s(L"WSARecv ERROR, Error Code: %d\n", err);

		DecrementProc(session);

		return false;
	}

	return true;
}

void errorLog(const WCHAR* s)
{
	int err = WSAGetLastError();

	wprintf_s(L"%s, Error Code: %d\n", s, err);
}
