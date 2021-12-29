#pragma comment(lib, "ws2_32")

#define SERVER_PORT (10170)

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdio.h>

SOCKET g_clientSocket;

int main()
{
	WSADATA wsa;
	//WCHAR mServerIP[32] = L"106.245.38.107";
	WCHAR mServerIP[32] = L"127.0.0.1";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup Fail\n");
		return -1;
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	InetPton(AF_INET, mServerIP, &addr.sin_addr);

	g_clientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (g_clientSocket == SOCKET_ERROR)
	{
		printf("CREATE Socket Fail\n");
		return -1;
	}

	u_long on = 1;
	int retval = ioctlsocket(g_clientSocket, FIONBIO, &on);
	if (retval == SOCKET_ERROR)
	{
		printf("Transfer Non Block Socket Fail\n");
		return -1;
	}

	int conVal = connect(g_clientSocket, (SOCKADDR*)&addr, sizeof(addr));

	if (conVal == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		int optval = 0;
		int len = sizeof(optval);
		int a;

		if (err == WSAEWOULDBLOCK)
		{
			while (1)
			{
				int temp = getsockopt(g_clientSocket, SOL_SOCKET, SO_ERROR, (char*)&optval, &len);
				//optval = WSAGetLastError();

				switch (optval)
				{
				case WSAEWOULDBLOCK:
					a = 0;
					break;
				case WSAETIMEDOUT:
					a = 1;
					break;
				case WSAECONNREFUSED:
					a = 2;
					break;
				default:
					a = 3;
					break;
				}
			}
		}

		return false;
	}

	return 0;
}