#pragma comment(lib, "ws2_32")

#define dfNETWORK_PORT (10170)
#define dfBACKLOG_SIZE (0)

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdio.h>

SOCKET g_listenSocket;

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup Fail\n");
		return -1;
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(dfNETWORK_PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	g_listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_listenSocket == SOCKET_ERROR)
	{
		printf("CREATE Socket Fail\n");
		return -1;
	}

	int bindval = bind(g_listenSocket, (SOCKADDR*)&addr, sizeof(addr));

	if (bindval == SOCKET_ERROR)
	{
		printf("Bind Fail\n");
		return -1;
	}

	int listenVal = listen(g_listenSocket, dfBACKLOG_SIZE);

	if (listenVal == SOCKET_ERROR)
	{
		printf("Socket Listen Fail\n");
		return -1;
	}

	while (1)
	{
		int t = 0;
	}

	SOCKADDR_IN clientAddr;
	int len = sizeof(clientAddr);
	SOCKET client = accept(g_listenSocket, (SOCKADDR*)&clientAddr, &len);

	if (client == INVALID_SOCKET)
	{
		printf("Socket Accept Fail\n");
		return -1;
	}

	int test = 0;

	return 0;
}