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
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}
	/// <summary>
	/// Create UDP Socket
	/// </summary>
	/// <returns> socket distributor </returns>
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Socket Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		return 1;
	}

	/// <summary>
	/// Socket BroadCasting setting
	/// </summary>
	/// <returns> error info </returns>
	bool bEnable = true;
	int retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&bEnable, sizeof(bEnable));
	if (retval == SOCKET_ERROR)
	{
		printf("Socket Option Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		closesocket(sock);
		return 1;
	}

	/// <summary>
	/// Receive Time out setting
	/// </summary>
	/// <returns> error info </returns>
	int interval = 200;
	retval = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&interval, sizeof(interval));
	if (retval == SOCKET_ERROR)
	{
		printf("Socket Option Error\n");
		printf("Error Code %u: \n", WSAGetLastError());
		closesocket(sock);
		return 1;
	}

	SOCKADDR_IN remote;
	memset(&remote, 0, sizeof(remote));

	remote.sin_family = AF_INET;
	InetPton(AF_INET, L"255.255.255.255", &remote.sin_addr.S_un.S_addr);

	MyHeader header = { 0xaa, 0xbb, 0xcc, 0xdd };

	for (int i = 10001; i < 10100; ++i)
	{
		WCHAR buffer[256] = { 0, };
		SOCKADDR_IN peerAddr;
		int addrLen = sizeof(peerAddr);

		remote.sin_port = htons(i);

		retval = sendto(sock, (char*)&header, sizeof(header), 0, (SOCKADDR*)&remote, sizeof(remote));

		if (retval == SOCKET_ERROR) {
			printf("send error\n");
			printf("Error Code: %d\n", WSAGetLastError());
			closesocket(sock);
			return 1;
		}

		retval = recvfrom(sock, (char*)buffer, sizeof(buffer), 0, (SOCKADDR*)&peerAddr, &addrLen);

		WCHAR otherIP[16] = { 0, };
		WCHAR* pName = (WCHAR*)buffer;

		if (retval > 0) {
			InetNtop(AF_INET, &peerAddr.sin_addr, otherIP, sizeof(otherIP));
			if (retval > 0) {
				wprintf_s(L"IP: %s\n", otherIP);
				wprintf_s(L"Port: %d\n", ntohs(peerAddr.sin_port));
				wprintf_s(L"Name: %s\n", pName);
			}
		}
	}
	
	closesocket(sock);

	WSACleanup();

	return 0;
}