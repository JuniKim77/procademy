#pragma comment(lib, "ws2_32")

#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <time.h>
#include "RingBuffer.h"

#define SOCKET_MAX (5)
#define WIDTH (500)

// 헤더
#pragma pack(push, 1)
struct stHEADER
{
	unsigned short Len;
};

// 패킷
struct st_DRAW_PACKET
{
	int		iStartX;
	int		iStartY;
	int		iEndX;
	int		iEndY;
};
#pragma pack(pop)

void logError(const WCHAR* msg);

SOCKET sockets[SOCKET_MAX];
RingBuffer sendBuffers[SOCKET_MAX];

int main()
{
	srand((unsigned)time(NULL));
	
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKADDR_IN server;
	server.sin_family = AF_INET;
	server.sin_port = htons(25000);
	InetPton(AF_INET, L"127.0.0.1", &server.sin_addr);

	for (int i = 0; i < SOCKET_MAX; ++i)
	{
		sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
		if (sockets[i] == INVALID_SOCKET) {
			logError(L"socket error");
		}

		int retConnect = connect(sockets[i], (SOCKADDR*)&server, sizeof(server));
		if (retConnect == SOCKET_ERROR) {
			logError(L"connect error");
		}

		u_long on = 1;
		int sockoptRet = ioctlsocket(sockets[i], FIONBIO, &on);
		if (sockoptRet == SOCKET_ERROR) {
			logError(L"NON-BLOCK error");
		}
	}

	while (1) {
		for (int i = 0; i < SOCKET_MAX; ++i)
		{
			if (sendBuffers[i].GetFreeSize() < 36) {
				continue;
			}

			stHEADER header;
			st_DRAW_PACKET packet;
			header.Len = sizeof(packet);

			int x = rand() % WIDTH;
			int y = rand() % WIDTH;

			packet.iStartX = x;
			packet.iStartY = 0;
			packet.iEndX = WIDTH - x;
			packet.iEndY = WIDTH;

			sendBuffers[i].Enqueue((char*)&header, sizeof(header));
			sendBuffers[i].Enqueue((char*)&packet, sizeof(packet));

			packet.iStartX = 0;
			packet.iStartY = y;
			packet.iEndX = WIDTH;
			packet.iEndY = WIDTH - y;

			sendBuffers[i].Enqueue((char*)&header, sizeof(header));
			sendBuffers[i].Enqueue((char*)&packet, sizeof(packet));
		}

		for (int i = 0; i < SOCKET_MAX; ++i)
		{
			char buffer[3000];

			int peekSize = sendBuffers[i].Peek(buffer, sendBuffers[i].GetUseSize());

			int sendSize = send(sockets[i], buffer, peekSize, 0);

			if (sendSize == SOCKET_ERROR) {
				int err = WSAGetLastError();

				if (err == WSAEWOULDBLOCK) {
					goto SEND;
				}

				logError(L"socket error");
			}

			sendBuffers[i].MoveFront(sendSize);

			SEND:

			int recvRet = recv(sockets[i], buffer, 3000, 0);

			if (recvRet == SOCKET_ERROR) {
				int err = WSAGetLastError();

				if (err == WSAEWOULDBLOCK) {
					continue;
				}

				logError(L"socket error");
			}
		}

		Sleep(10);
	}

	return 0;
}

void logError(const WCHAR* msg)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s code : %d\n", msg, err);

	exit(1);
}