#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

struct MyHeader
{
	char h1;
	char h2;
	char h3;
	char h4;
};

int main()
{
	setlocale(LC_ALL, "");
	int retval;

	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	// socket
	SOCKET listen_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (listen_socket == INVALID_SOCKET) {
		int err = WSAGetLastError();
		printf("Error code: %d\n", err);
		return 1;
	}

	// socket address setting
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(10010);

	// bind
	retval = bind(listen_socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("Error code: %d\n", err);
		closesocket(listen_socket);
		return 1;
	}

	int interval = 200;
	retval = setsockopt(listen_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(interval));
	if (retval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("Error code: %d\n", err);
		closesocket(listen_socket);
		return 1;
	}

	MyHeader header = { 0xaa, 0xbb, 0xcc, 0xdd };
	SOCKADDR_IN clientAddr;
	int clientAddrLen;
	char buffer[32];

	WCHAR message[64] = L"스타크래프트";

	while (1)
	{
		clientAddrLen = sizeof(clientAddr);
		memset(buffer, 0, sizeof(buffer));
		retval = recvfrom(listen_socket, buffer, sizeof(buffer), 0, (SOCKADDR*)&clientAddr, &clientAddrLen);

		if (retval > 0)
		{
			WCHAR otherIP[16] = { 0, };
			InetNtop(AF_INET, &clientAddr.sin_addr, otherIP, sizeof(otherIP));

			wprintf_s(L"[UDP] Client IP: %s\n", otherIP);

			if (*((int*)&buffer) == *((int*)&header)) {
				wprintf_s(L"Code Access Pass\nSend Message: %s\n", message);

				retval = sendto(listen_socket, (char*)message, wcslen(message) * 2, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));

				if (retval == SOCKET_ERROR) {
					printf("send error\n");
					printf("Error Code: %d\n", WSAGetLastError());
				}
			}
		}
	}


	WSACleanup();

	return 0;
}