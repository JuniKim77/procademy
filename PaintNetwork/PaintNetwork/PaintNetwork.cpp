// PaintNetwork.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include "framework.h"
#include "PaintNetwork.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <locale.h>
#include <windowsx.h>
#include <iostream>
#include "RingBuffer.h"

#define MAX_LOADSTRING 100
#define PORT (25000)
#define WM_SOCKET (WM_USER+1)

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
HWND g_MainWindow;
SOCKET g_sock;
bool gb_connected = false;
RingBuffer g_recv_buffer(MAX_LOADSTRING);
RingBuffer g_send_buffer(MAX_LOADSTRING);

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

SOCKET ConnectNetwork(HWND hWnd);
bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);
void OpenConsole();
void logError(const WCHAR* msg);
void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SendRingBuffer();
void recvMessageProc(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	setlocale(LC_ALL, "");

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	// 애플리케이션 초기화를 수행합니다:
	if (CreateMainWindow(hInstance, L"Paint", L"PaintNetwork") == false)
		return -1;

	MSG msg;

	OpenConsole();
	system("mode con: cols=50 lines=5");

	g_sock = ConnectNetwork(g_MainWindow);

	// 기본 메시지 루프입니다:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int x;
	static int y;
	static BOOL NowDraw;

	switch (message)
	{
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		NowDraw = TRUE;
		break;
	case WM_LBUTTONUP:
		NowDraw = FALSE;
		break;
	case WM_MOUSEMOVE:
	{
		if (gb_connected == false) {
			break;
		}
		if (NowDraw == TRUE) {
			// send Packet
			int new_x = LOWORD(lParam);
			int new_y = HIWORD(lParam);

			stHEADER header;
			st_DRAW_PACKET packet = { x, y, new_x, new_y };
			header.Len = sizeof(st_DRAW_PACKET);
			g_send_buffer.Enqueue((char*)&header, sizeof(header));
			g_send_buffer.Enqueue((char*)&packet, sizeof(packet));

			SendRingBuffer();

			x = new_x;
			y = new_y;
		}
		break;
	}
	case WM_SOCKET:
	{
		SocketMessageProc(hWnd, message, wParam, lParam);
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

SOCKET ConnectNetwork(HWND hWnd)
{
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	WCHAR serverIP[16];
	wprintf_s(L"접속 IP: ");
	_getws_s(serverIP, 16);
	InetPton(AF_INET, serverIP, &serverAddr.sin_addr);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		logError(L"소켓 생성 에러");
	}

	int asyncselectRetval = WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);
	if (asyncselectRetval == SOCKET_ERROR)
		logError(L"소켓 이벤트 등록 실패");

	int conretval = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (conretval == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			logError(L"연결 에러");
		}
	}
	return sock;
}

bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINTNETWORK));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_PAINTNETWORK);
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// 애플리케이션 초기화를 수행합니다:
	HWND hWnd = CreateWindowExW(0, className, windowName, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
		return false;

	g_MainWindow = hWnd;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.top = 0;
	WindowRect.right = 640;
	WindowRect.bottom = 480;

	AdjustWindowRectEx(&WindowRect, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL,
		GetWindowStyle(hWnd));

	int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (640 / 2);
	int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (480 / 2);

	MoveWindow(hWnd, x, y, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);

	return true;
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

void logError(const WCHAR* msg)
{
	int err = WSAGetLastError();
	wprintf_s(L"%s code : %d\n", msg, err);

	exit(1);
}

void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam)) 
	{
		logError(L"WSAGETSELECTERROR 에러");
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:
	{
		while (1)
		{
			char buffer[3000];

			int recvSize = recv(g_sock, buffer, g_recv_buffer.GetFreeSize(), 0);

			if (recvSize == SOCKET_ERROR)
			{
				int err = WSAGetLastError();

				if (err == WSAEWOULDBLOCK)
				{
					break;
				}

				logError(L"recv 에러");
			}

			int enSize = g_recv_buffer.Enqueue(buffer, recvSize);

			if (g_recv_buffer.GetFreeSize() == 0)
				break;
		}

		recvMessageProc(hWnd);
		break;
	}
	case FD_WRITE:
		wprintf_s(L"쓰기 가능!\n");
		SendRingBuffer();
		break;
	case FD_CLOSE:
		logError(L"접속 종료");
		break;
	case FD_CONNECT:
		wprintf_s(L"연결 성공\n");
		gb_connected = true;
		break;
	default:
		break;
	}
}

void SendRingBuffer()
{
	while (1)
	{
		char buffer[3000];

		int peekSize = g_send_buffer.Peek(buffer, 3000);

		if (peekSize == 0)
			break;

		int sendSize = send(g_sock, buffer, peekSize, 0);

		if (sendSize == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err == WSAEWOULDBLOCK)
			{
				break;
			}

			logError(L"send Error");
		}

		g_send_buffer.MoveFront(sendSize);

		if (sendSize < peekSize)
			break;
	}
}

void recvMessageProc(HWND hWnd)
{
	HDC hdc = GetDC(hWnd);
	// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
	while (1)
	{
		if (g_recv_buffer.GetUseSize() < sizeof(stHEADER))
			break;

		stHEADER header;
		int peekSize = g_recv_buffer.Peek((char*)&header, sizeof(stHEADER));

		if (peekSize < sizeof(header))
			break;

		if (g_recv_buffer.GetUseSize() < (sizeof(stHEADER) + header.Len))
			break;

		g_recv_buffer.MoveFront(sizeof(stHEADER));
		st_DRAW_PACKET packet;
		g_recv_buffer.Dequeue((char*)&packet, header.Len);

		MoveToEx(hdc, packet.iStartX, packet.iStartY, nullptr);

		LineTo(hdc, packet.iEndX, packet.iEndY);
	}
	ReleaseDC(hWnd, hdc);
}
