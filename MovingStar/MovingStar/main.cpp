#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "process.h"

#define SERVER_PORT (3000)

int main()
{
	setlocale(LC_ALL, "");
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	SOCKET sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		wprintf_s(L"��Ĺ ���� ���� Code: %d\n", WSAGetLastError());
		return 1;
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	
	WCHAR server_IP[16] = { 0, };
	wprintf_s(L"������ IP �ּҸ� �Է��ϼ���. : ");
	if (fgetws(server_IP, 16, stdin) == nullptr)
		wprintf_s(L"�Է� ����\n");
	
	InetPton(AF_INET, server_IP, &addr.sin_addr);

	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		wprintf_s(L"���� ���� Code: %d\n", WSAGetLastError());
		return 1;
	}

	wprintf_s(L"���� ����!\n");

	u_long on = 1;
	if (ioctlsocket(sock, FIONBIO, &on) == SOCKET_ERROR)
	{
		wprintf_s(L"���� ���� ���� Code: %d\n", WSAGetLastError());
		closesocket(sock);
		return 1;
	}
	ScreenInitial();

	while (1)
	{
		// Ű �Է�
		KeyProcess();
		// �޼��� ó��
		NetworkProcess(sock);
		// ������
		Render();
	}
}