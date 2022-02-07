#pragma comment(lib, "ws2_32")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <iostream>
#include <conio.h>

using namespace std;

#define SERVER_PORT (10170)

int main()
{
    WSADATA			wsa;
    WCHAR mServerIP[32] = L"127.0.0.1";

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return -1;
    }

    while (1)
    {
        SOCKET cli = socket(AF_INET, SOCK_STREAM, 0);

        /* u_long on = 1;
        int retval = ioctlsocket(cli, FIONBIO, &on);
        if (retval == SOCKET_ERROR)
        {
            printf("Transfer Non Block Socket Fail\n");
            return -1;
        }*/

        SOCKADDR_IN addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_PORT);
        InetPton(AF_INET, mServerIP, &addr.sin_addr);

        if (cli == SOCKET_ERROR)
        {
            printf("CREATE Socket Fail\n");
            return -1;
        }

        int conVal = connect(cli, (SOCKADDR*)&addr, sizeof(addr));

        if (conVal == SOCKET_ERROR)
        {
            int err = WSAGetLastError();

            cout << "connect error : " << err << endl;

            break;
        }
    }

    while (1)
    {
        int a = 0;
    }

    return 0;
}