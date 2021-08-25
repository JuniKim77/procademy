#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT (9000)
#define BUFSIZE (512)

struct SOCKETINFO
{
	WSAOVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuf;
};

int g_nTotalSockets = 0;
SOCKETINFO g_SocketInfoArray[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION cs;

DWORD WINAPI WorkerThread(LPVOID arg);
bool AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int index);

int main(int argc, char* argv[])
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET) return 1;

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	int retval = bind(listen_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) return 1;

	retval = listen(listen_socket, SOMAXCONN);

	if (retval == SOCKET_ERROR) return 1;

	WSAEVENT hEvent = WSACreateEvent(); // 더미 이벤트
	if (hEvent == WSA_INVALID_EVENT)
		return 1;

	EventArray[g_nTotalSockets] = hEvent;

	HANDLE hThread = CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
	if (hThread == NULL) return 1;
	CloseHandle(hThread);

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes;
	DWORD flags;
	
	while (1) 
	{

	}

	return 0;
}

DWORD __stdcall WorkerThread(LPVOID arg)
{
	return 0;
}

bool AddSocketInfo(SOCKET sock)
{
	return false;
}

void RemoveSocketInfo(int index)
{
}
