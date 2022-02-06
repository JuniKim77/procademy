#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <conio.h>

#define SERVER_PORT (10170)

using namespace std;

int main()
{
    WSADATA			wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        cout << "wsa error" << endl;
        return -1;
    }

    SOCKADDR_IN addr;
    SOCKET listenSocket;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
    {
        int err = WSAGetLastError();

        cout << "create socket error : " << err << endl;

        return -1;
    }

    u_long on = 1;
    int retval = ioctlsocket(listenSocket, FIONBIO, &on);
    if (retval == SOCKET_ERROR)
    {
        printf("Transfer Non Block Socket Fail\n");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();

        cout << "bind error : " << err << endl;
        return -1;
    }


    //if (listen(listenSocket, SOMAXCONN_HINT(1000)) == SOCKET_ERROR)
    if (listen(listenSocket, 5) == SOCKET_ERROR)
    {
        int err = WSAGetLastError();

        cout << "listen error : " << err << endl;
        return -1;
    }

    while (1)
    {
        char c = _getch();

        if (c == 's')
        {
            break;
        }
    }

    int count = 0;

    while (1)
    {
        SOCKADDR_IN client;
        int len = sizeof(client);
        SOCKET cli = accept(listenSocket, (SOCKADDR*)&client, &len);

        if (cli == SOCKET_ERROR)
        {
            int err = WSAGetLastError();

            if (err == WSAEWOULDBLOCK)
            {
                break;
            }
        }
        else
        {
            count++;

            cout << "Accept count : " << count << endl;
        }
    }

	return 0;
}