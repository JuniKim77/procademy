// ActionGame.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#pragma comment(lib, "imm32.lib")

#define WINDOW_WIDTH (640)
#define WINDOW_HEIGHT (480)
#define WINDOW_COLORBIT (32)

#include "framework.h"
#include "ActionGame.h"
#include <windowsx.h>
#include "GameProcess.h"
#include "ScreenDib.h"
#include "SpriteDib.h"
#include "ESprite.h"
#include <stdio.h>
#include <locale.h>
#include <Windows.h>
#include <timeapi.h>
#include "FrameSkip.h"
#include "Session.h"

#define WM_SOCKET (WM_USER + 1)

#pragma comment(lib, "winmm.lib")

// 전역 변수:
ScreenDib gScreenDib(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_COLORBIT);
SpriteDib gSpriteDib(eSPRITE_MAX, 0x00ffffff);
HWND gMainWindow;
bool gbActiveApp;
HIMC gOldImc;
FrameSkip gFrameSkipper;
Session g_session;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void OpenConsole();
void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	timeBeginPeriod(1);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	OpenConsole();

	gFrameSkipper.CheckTime();
	gFrameSkipper.Reset();

	// TODO: 여기에 코드를 입력합니다.
	InitializeGame();

	if (CreateMainWindow(hInstance, L"MainWindow", L"ActionGame") == false)
		return 1;

	if (!g_session.Connect(gMainWindow))
		return 1;

	ContentLoad();

	MSG msg;

	while (1)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (g_session.IsConnected()) {
				RunGame();
			}
		}
	}

	FreeConsole();
	timeEndPeriod(1);

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
	switch (message)
	{
	case WM_SOCKET:
		SocketMessageProc(hWnd, message, wParam, lParam);
		break;
	case WM_CREATE:
		gOldImc = ImmAssociateContext(hWnd, nullptr);
		break;
	case WM_ACTIVATEAPP:
		gbActiveApp = (bool)wParam;
		break;
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		ImmAssociateContext(hWnd, gOldImc);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void OpenConsole()
{
	setlocale(LC_ALL, "");

	FILE* fin;
	FILE* fout;
	FILE* ferr;

	if (AllocConsole())
	{
		freopen_s(&fin, "CONIN$", "r", stdin);
		freopen_s(&ferr, "CONOUT$", "w", stderr);
		freopen_s(&fout, "CONOUT$", "w", stdout);
	}
}

void SocketMessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		g_session.ErrorQuit(L"Select 에러");
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		g_session.mbConnected = true;
		break;
	case FD_CLOSE:
		g_session.Disconnect();
		g_session.ErrorQuit(L"접속 종료");
		break;
	case FD_READ:
		g_session.ReceivePacket();
		g_session.recvProc();
		break;
	case FD_WRITE:
		g_session.writeProc();
		break;
	}
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ACTIONGAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ACTIONGAME);
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// 애플리케이션 초기화를 수행합니다:
	HWND hWnd = CreateWindowExW(0, className, windowName, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
		return false;

	gMainWindow = hWnd;

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