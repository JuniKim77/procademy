#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0); // 소켓 디스크립터 반환
	if (sock == INVALID_SOCKET) {
		printf("Socket Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}
	bool bEnable = true;
	int retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));
	if (retval == SOCKET_ERROR)
	{
		printf("Socket Option Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}

	int interval = 200;

	retval = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(interval));
	if (retval == SOCKET_ERROR)
	{
		printf("Socket Option Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}

	SOCKADDR_IN remote;
	memset(&remote, 0, sizeof(remote));

	remote.sin_family = AF_INET;
	


	return 0;
}