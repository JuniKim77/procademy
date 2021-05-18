#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "session.h"
#include <locale.h>

#define SERVER_PORT (3000)
#define MAX_SESSION (60)

void NetworkSetting();
SOCKET g_listen_socket;
Session g_sessions[MAX_SESSION];
int g_num_ID = 1;

/// <summary>
/// �������� ��Ʈ��ũ ó��
/// </summary>
void networkProc();

/// <summary>
/// ������ �Լ�
/// </summary>
void render();

/// <summary>
/// ������ �޾Ƶ��̴� �Լ�
/// </summary>
void acceptProc();

/// <summary>
/// ���� �޼����� ������ ���鿡�� �ѷ��ִ� �Լ�
/// </summary>
/// <param name="to"></param>
void receiveProc(Session* from);

/// <summary>
/// �ش� ���ǿ� �޼����� ������ �Լ�
/// </summary>
/// <param name="to"></param>
/// <param name="packet"></param>
void sendUniCast(Session* to, char* packet);

/// <summary>
/// except�� ������ ��� ���ǿ� �޼����� �Ѹ��� �Լ�
/// null�� ��� ���״�� ��ε�ĳ����
/// </summary>
/// <param name="except"></param>
/// <param name="packet"></param>
void sendBroadCast(Session* except, char* packet);

/// <summary>
/// ������ �����ϰ� ���� �����͸� �����ϴ� �Լ�
/// ���� Ư����, �� ���� �޼����� �ѷ��ִ� �۾�����
/// </summary>
/// <param name="to"></param>
void disconnect(Session* obj);

int main()
{
	setlocale(LC_ALL, "");

	NetworkSetting();

	while (1)
	{
		networkProc();

		// render();
	}


	return 0;
}

void NetworkSetting()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit(1);
	}

	g_listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_listen_socket == INVALID_SOCKET) {
		int err = WSAGetLastError();
		wprintf_s(L"���� ���� : %d\n", err);
		exit(1);
	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(g_listen_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		wprintf_s(L"���ε� ���� : %d\n", err);
		exit(1);
	}

	u_long on = 1;
	if (ioctlsocket(g_listen_socket, FIONBIO, &on) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		wprintf_s(L"���� ���� ��ȯ ���� : %d\n", err);
		exit(1);
	}

	if (listen(g_listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		wprintf_s(L"������ ���� : %d\n", err);
		exit(1);
	}

	// wprintf_s(L"���� ������ ����\n");
}

void networkProc()
{
	FD_SET rset;
	FD_ZERO(&rset);

	FD_SET(g_listen_socket, &rset);

	for (int i = 0; i < MAX_SESSION; ++i)
	{
		if (g_sessions[i].socket != INVALID_SOCKET) 
		{
			FD_SET(g_sessions[i].socket, &rset);
		}
	}

	int retval = select(0, &rset, NULL, NULL, NULL);

	if (retval <= 0) 
	{
		int err = WSAGetLastError();
		wprintf_s(L"Select ���� : %d\n", err);
		exit(1);
	}
	
	if (FD_ISSET(g_listen_socket, &rset))
	{
		acceptProc();
	}

	for (int i = 0; i < MAX_SESSION; ++i)
	{
		if (g_sessions[i].socket == INVALID_SOCKET)
			continue;

		if (FD_ISSET(g_sessions[i].socket, &rset))
		{
			// wprintf_s(L"receive ����\n");
			receiveProc(&g_sessions[i]);
		}
	}
}

void render()
{
}

void acceptProc()
{
	while (1)
	{
		SOCKADDR_IN clientAddr;
		int len = sizeof(clientAddr);

		SOCKET client = accept(g_listen_socket, (SOCKADDR*)&clientAddr, &len);

		if (client == INVALID_SOCKET) 
		{
			int err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK)
			{
				break;
			}

			wprintf_s(L"Accept error : %d\n", err);
			continue;
		}

		u_long on = 1;
		if (ioctlsocket(client, FIONBIO, &on) == SOCKET_ERROR) {
			int err = WSAGetLastError();
			wprintf_s(L"���� ���� ��ȯ ���� : %d\n", err);
			closesocket(client);
			continue;
		}

		// Search for an empty session
		bool found = false;
		for (int i = 0; i < MAX_SESSION; ++i)
		{
			if (g_sessions[i].socket == INVALID_SOCKET)
			{
				g_sessions[i].socket = client;
				g_sessions[i].port = ntohs(clientAddr.sin_port);
				g_sessions[i].IP = clientAddr.sin_addr.S_un.S_addr;
				g_sessions[i].ID = g_num_ID;
				g_sessions[i].x = SCREEN_WIDTH / 2;
				g_sessions[i].y = SCREEN_HEIGTH / 2;

				int packet[4] = { 0, g_num_ID, };
				// ID �Ҵ� �޼��� 
				sendUniCast(&g_sessions[i], (char*)&packet);
				
				packet[0] = 1;
				packet[2] = g_sessions[i].x;
				packet[3] = g_sessions[i].y;
				// Sned a create message of myself
				sendBroadCast(nullptr, (char*)&packet);
				
				// send create messages of other stars
				for (int j = 0; j < MAX_SESSION; ++j)
				{
					if (g_sessions[j].socket != INVALID_SOCKET && g_sessions[j].socket != g_sessions[i].socket)
					{
						wprintf_s(L"�����ֱ� ID: %d\n", g_sessions[j].ID);
						packet[1] = g_sessions[j].ID;
						packet[2] = g_sessions[j].x;
						packet[3] = g_sessions[j].y;
						sendUniCast(&g_sessions[i], (char*)&packet);
					}
				}

				g_num_ID++;
				found = true;
				break;
			}
		}

		if (!found) {
			wprintf_s(L"���� ���� ������ �� á���ϴ�.\n");
			closesocket(client);

			break;
		}
			
		// wprintf_s(L"���� ����!\n");
		WCHAR clientIP[16];
		InetNtop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
		wprintf_s(L"IP : %s, Port : %d\n", clientIP, ntohs(clientAddr.sin_port));
	}
}

void receiveProc(Session* from)
{
	while (1)
	{
		int packet[4];
		int retval = recv(from->socket, (char*)&packet, 16, 0);

		if (retval == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK)
			{
				return;
			}

			disconnect(from);

			return;
		}

		int x = packet[2];
		int y = packet[3];

		if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGTH)
		{
			// wprintf_s(L"ID, %d�� ���ָ� ��Ż�Ϸ� ��\n", from->ID);
			continue;
		}

		// x, y ��ǥ ������Ʈ
		for (int i = 0; i < MAX_SESSION; ++i)
		{
			if (g_sessions[i].ID == packet[1])
			{
				g_sessions[i].x = x;
				g_sessions[i].y = y;
				break;
			}
		}
		sendBroadCast(from, (char*)&packet);
		// wprintf_s(L"���ú� ����\n");
	}
}

void sendUniCast(Session* to, char* packet)
{
	if (to->socket == INVALID_SOCKET)
		return;

	if (send(to->socket, packet, 16, 0) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf_s(L"Send Unicast Error : %d\n", err);

		disconnect(to);
	}

	// wprintf_s(L"Send Unit ����\n");
}

void sendBroadCast(Session* except, char* packet)
{
	if (except == nullptr)
	{
		for (int i = 0; i < MAX_SESSION; ++i)
		{
			if (g_sessions[i].socket == INVALID_SOCKET)
				continue;

			if (send(g_sessions[i].socket, packet, 16, 0) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				wprintf_s(L"Send Broadcast Error : %d\n", err);

				disconnect(&g_sessions[i]);
			}
		}
		// wprintf_s(L"Send Broad cast ����\n");
		return;
	}
	
	for (int i = 0; i < MAX_SESSION; ++i)
	{
		if (g_sessions[i].socket != except->socket)
		{
			if (g_sessions[i].socket == INVALID_SOCKET)
				continue;

			if (send(g_sessions[i].socket, packet, 16, 0) == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				wprintf_s(L"Send Broadcast Error : %d\n", err);

				disconnect(&g_sessions[i]);
			}
		}
	}

	// wprintf_s(L"Send Broad cast ����\n");
}

void disconnect(Session* obj)
{
	wprintf_s(L"disconnect:  ");
	obj->printInfo();
	closesocket(obj->socket);
	obj->socket = INVALID_SOCKET;

	int packet[4] = { 2, obj->ID };

	sendBroadCast(nullptr, (char*)&packet);
}
