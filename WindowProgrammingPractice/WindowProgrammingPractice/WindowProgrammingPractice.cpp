// WindowProgrammingPractice.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WindowProgrammingPractice.h"
#include <windowsx.h>
#include <stdlib.h>
#include <time.h>

// 전역 변수:
HPEN gPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

#define QUEUE_SIZE (100)

int queue[QUEUE_SIZE];
int queueRear = 0;
int queueFront = 0;
int queueSize = 0;
int enqueue(int value);
void popqueue();
int peakqueue(int pos);
// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWPROGRAMMINGPRACTICE));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWPROGRAMMINGPRACTICE);
	wcex.lpszClassName = L"ABC";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	HWND hWnd = CreateWindowW(L"ABC", L"프로카데미", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		SetTimer(hWnd, 1, 50, nullptr);
		SelectObject(hdc, gPen);
		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 1:
		{
			int y = 250 + rand() % 200;

			enqueue(y);
			InvalidateRect(hWnd, nullptr, true);
			break;
		}
		default:
			break;
		}
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
		int y = peakqueue(queueFront);
		int count = 0;
		SelectObject(hdc, gPen);
		MoveToEx(hdc, count, y, nullptr);

		for (int i = queueFront; i != queueRear; i = (i + 1) % QUEUE_SIZE)
		{
			count += 10;
			y = peakqueue(i);
			LineTo(hdc, count, y);
		}

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

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

int enqueue(int value)
{
	queue[queueRear] = value;

	queueRear = (queueRear + 1) % QUEUE_SIZE;

	if (queueFront == queueRear)
	{
		queueFront = (queueFront + 1) % QUEUE_SIZE;
	}

	return 0;
}

void popqueue()
{
	queueFront = (queueFront + 1) % QUEUE_SIZE;
}

int peakqueue(int pos)
{
	return queue[pos];
}
