// PaintNetworkServer.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include "framework.h"
#include "PaintNetworkServer.h"
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Session.h"

#define MAX_LOADSTRING 100
#define PORT (25000)
#define WM_SOCKET (WM_USER+1)
#define MAX_SESSION (100)

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

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
SOCKET g_listen_socket;
Session g_sessions[MAX_SESSION];

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void OpenConsole();
SOCKET socketInit(HWND hDlg);
void ErrorQuit(const WCHAR* msg);
void ErrorDisplay(const WCHAR* msg);
void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Disconnect(Session* session);
Session* SearchEmptySession();
Session* SearchSession(SOCKET socket);
void SendRingBuffer(Session* session);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    OpenConsole();

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);

    return 0;
}

LRESULT DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        g_listen_socket = socketInit(hDlg);
        break;
    }
    case WM_SOCKET:
    {
        SocketMessageProc(hDlg, uMsg, wParam, lParam);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return 0;
        default:
            break;
        }

    default:
        break;
    }
    return 0;
}

void OpenConsole()
{
    FILE* fout;
    FILE* fin;
    FILE* ferr;

    if (AllocConsole())
    {
        freopen_s(&fin, "CONIN$", "r", stdin);
        freopen_s(&ferr, "CONOUT$", "w", stderr);
        freopen_s(&fout, "CONOUT$", "w", stdout);
    }
}

SOCKET socketInit(HWND hDlg)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        ErrorQuit(L"WSAStartup error");

    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        ErrorQuit(L"socket create error");

    int bindRet = bind(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (bindRet == SOCKET_ERROR)
        ErrorQuit(L"Bind Error");

    int listenRet = listen(sock, SOMAXCONN);
    if (listenRet == SOCKET_ERROR)
        ErrorQuit(L"listen error");

    int asyncSelectRet = WSAAsyncSelect(sock, hDlg, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
    if (asyncSelectRet == SOCKET_ERROR)
        ErrorQuit(L"WSAAsyncSelect error");

    return sock;
}

void ErrorQuit(const WCHAR* msg)
{
    int err = WSAGetLastError();
    wprintf_s(L"%s code : %d\n", msg, err);

    exit(1);
}

void ErrorDisplay(const WCHAR* msg)
{
    int err = WSAGetLastError();
    wprintf_s(L"%s code : %d\n", msg, err);
}

void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WSAGETSELECTERROR(lParam))
    {
        ErrorQuit(L"WSAGETSELECTERROR 에러");
    }

    switch (WSAGETSELECTEVENT(lParam))
    {
    case FD_ACCEPT:
    {
        while (1)
        {
            SOCKADDR_IN clientAddr;
            int size = sizeof(clientAddr);

            int clientSock = accept(wParam, (SOCKADDR*)&clientAddr, &size);
            if (clientSock == INVALID_SOCKET)
            {
                int err = WSAGetLastError();

                if (err == WSAEWOULDBLOCK)
                    break;

                ErrorDisplay(L"접속 에러");
                continue;
            }

            int retval = WSAAsyncSelect(clientSock, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
            if (retval == SOCKET_ERROR) 
            {
                ErrorDisplay(L"접속 에러");
                closesocket(clientSock);
                continue;
            }

            Session* session = SearchEmptySession();
            if (session == nullptr) {
                ErrorDisplay(L"접속 한도 초과");
                break;
            }
            session->socket = clientSock;
            session->IP = clientAddr.sin_addr.S_un.S_addr;
            session->port = clientAddr.sin_port;
        }
        break;
    }
    case FD_READ:
    {
        Session* session = SearchSession(wParam);

        while (1)
        {
            char buffer[10000];

            int recvSize = recv(session->socket, buffer, session->recv.GetFreeSize(), 0);

            if (recvSize == SOCKET_ERROR)
            {
                int err = WSAGetLastError();

                if (err == WSAEWOULDBLOCK)
                {
                    break;
                }

                Disconnect(session);
                break;
            }

            int enSize = session->recv.Enqueue(buffer, recvSize);

            if (session->recv.GetFreeSize() == 0)
                break;
        }

        SendRingBuffer(session);
        break;
    }
    case FD_WRITE:
    {
        Session* session = SearchSession(wParam);

        if (session != nullptr)
            SendRingBuffer(session);

        break;
    }
    case FD_CLOSE:
    {
        if (wParam == g_listen_socket) {
            ErrorQuit(L"접속 종료");
        }
        
        Session* session = SearchSession(wParam);
        if (session != nullptr) {
            Disconnect(session);
        }
        break;
    }
    default:
        break;
    }
}

void Disconnect(Session* session)
{
    closesocket(session->socket);
    session->socket = INVALID_SOCKET;
    session->recv.ClearBuffer();
    session->send.ClearBuffer();
    // 메세지
}

Session* SearchEmptySession()
{
    return SearchSession(INVALID_SOCKET);
}

Session* SearchSession(SOCKET socket)
{
    for (int i = 0; i < MAX_SESSION; ++i)
    {
        if (g_sessions[i].socket == socket)
            return &g_sessions[i];
    }

    return nullptr;
}

void SendRingBuffer(Session* session)
{
    while (1)
    {
        if (session->recv.GetUseSize() < sizeof(stHEADER))
            break;

        stHEADER header;
        int peekSize = session->recv.Peek((char*)&header, sizeof(header));

        if (peekSize < sizeof(header))
            break;

        if (session->recv.GetUseSize() < (sizeof(stHEADER) + header.Len))
            break;

        session->recv.MoveFront(sizeof(stHEADER));
        st_DRAW_PACKET packet;
        session->recv.Dequeue((char*)&packet, header.Len);



    }
}
