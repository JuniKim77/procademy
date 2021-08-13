#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>

#define BUFFSIZE (512)

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

int g_nTotalSockets = 0;
SOCKETINFO* g_socketInfoArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT g_eventArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION cs;

unsigned __stdcall WorkerThread(void* arg);
bool AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int index);
void error_quit(const WCHAR* s);

int main(int argc, char* argv[])
{
	InitializeCriticalSection(&cs);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
	{
		error_quit(L"Listen Socket Error");
		return -1;
	}

	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR)
	{
		error_quit(L"Bind Error");
		return -1;
	}

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
	{
		error_quit(L"Listen Error");
		return -1;
	}
	
	// 더미 이벤트 생성
	WSAEVENT hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		error_quit(L"Event Create Error");
		return -1;
	}

	HANDLE hThread = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, nullptr, 0, nullptr);

	if (hThread == NULL)
	{
		error_quit(L"Thread Create Error");
		return -1;
	}
	CloseHandle(hThread);

	while (1) 
	{
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			error_quit(L"Accept Error");
			continue;
		}

		wprintf_s(L"[TCP server] client connect\n");

		
	}
	
}

unsigned __stdcall WorkerThread(void* arg)
{
	while (1)
	{
		DWORD retval = WSAWaitForMultipleEvents(g_nTotalSockets, g_eventArray, false, WSA_INFINITE, false);

		if (retval == WSA_WAIT_FAILED) {
			error_quit(L"WaitForMulti error");
			continue;
		}

		retval -= WSA_WAIT_EVENT_0;
		WSAResetEvent(g_eventArray[retval]);

	}
	return 0;
}

bool AddSocketInfo(SOCKET sock)
{
	SOCKETINFO* pInfo;
	WSAEVENT hEvent;

	EnterCriticalSection(&cs);

	if (g_nTotalSockets >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		wprintf_s(L"[Error] socket is full\n");
		goto Exit;
	}

	pInfo = new SOCKETINFO;

	hEvent = WSACreateEvent();
	
	ZeroMemory(&(pInfo->overlapped), sizeof(pInfo->overlapped));
	pInfo->overlapped.hEvent = hEvent;
	pInfo->sock = sock;
	pInfo->recvbytes = 0;
	pInfo->sendbytes = 0;
	pInfo->wsabuf.buf = pInfo->buf;
	pInfo->wsabuf.len = BUFFSIZE;
	g_socketInfoArray[g_nTotalSockets] = pInfo;
	g_eventArray[g_nTotalSockets] = hEvent;
	g_nTotalSockets++;

	LeaveCriticalSection(&cs);
	return true;

Exit:
	LeaveCriticalSection(&cs);
	return false;
}

void RemoveSocketInfo(int index)
{
	EnterCriticalSection(&cs);

	SOCKETINFO* pInfo = g_socketInfoArray[index];
	closesocket(pInfo->sock);
	delete pInfo;
	WSACloseEvent(g_eventArray[index]);

	for (int i = index; i < g_nTotalSockets - 1; ++i)
	{
		g_socketInfoArray[i] = g_socketInfoArray[i + 1];
		g_eventArray[i] = g_eventArray[i + 1];
	}

	g_nTotalSockets--;

	LeaveCriticalSection(&cs);
}

void error_quit(const WCHAR* s)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s: %d\n", s, err);
}
