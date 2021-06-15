
// MonitorGraph.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "ServerMonitor.h"
#include <windowsx.h>
#include <stdlib.h>
#include <time.h>
#include "MonitorGraphUnit.h"

// 전역 변수:
HWND gMainWindow;
MonitorGraphUnit* p1;
MonitorGraphUnit* p2;
MonitorGraphUnit* p3;
MonitorGraphUnit* p4;
HINSTANCE g_hInst;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	g_hInst = hInstance;

	if (CreateMainWindow(hInstance, L"MainWindow", L"서버 모니터링") == false)
		return -1;

	MSG msg;

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
	switch (message)
	{
	case WM_CREATE:
		p1 = new MonitorGraphUnit(g_hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 10, 10, 200, 200);
		p2 = new MonitorGraphUnit(g_hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 220, 10, 200, 200);
		p3 = new MonitorGraphUnit(g_hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 430, 10, 400, 200);
		p4 = new MonitorGraphUnit(g_hInst, hWnd, MonitorGraphUnit::LINE_SINGLE, 10, 220, 300, 250);
		SetTimer(hWnd, 1, 100, NULL);
		break;

	case WM_TIMER:
		p1->InsertData(rand() % 100);
		p2->InsertData(rand() % 100);
		p3->InsertData(rand() % 100);
		p4->InsertData(rand() % 100);
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
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_SERVERMONITOR));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SERVERMONITOR);
	wcex.lpszClassName = className;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	// 애플리케이션 초기화를 수행합니다:
	HWND hWnd = CreateWindowExW(0, className, windowName, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 640, nullptr, nullptr, hInstance, nullptr);

	if (hWnd == nullptr)
		return false;

	gMainWindow = hWnd;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.top = 0;
	WindowRect.right = 1280;
	WindowRect.bottom = 640;

	AdjustWindowRectEx(&WindowRect, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL,
		GetWindowStyle(hWnd));

	int x = (GetSystemMetrics(SM_CXSCREEN) / 2) - (1280 / 2);
	int y = (GetSystemMetrics(SM_CYSCREEN) / 2) - (640 / 2);

	MoveWindow(hWnd, x, y, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);

	return true;
}