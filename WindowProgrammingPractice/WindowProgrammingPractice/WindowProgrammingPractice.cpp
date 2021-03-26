// WindowProgrammingPractice.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WindowProgrammingPractice.h"
#include <windowsx.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// 전역 변수:
HDC g_hMemDC;
HBITMAP g_hMemBitmap;
HBITMAP g_hMemBitmapOld;
HPEN gPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
HINSTANCE g_hInstace;
HWND g_child1 = nullptr;

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
	wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWPROGRAMMINGPRACTICE);
	wcex.lpszClassName = L"ABC";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

	g_hInstace = hInstance;

	HWND hWnd = CreateWindowW(L"ABC", L"프로카데미", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

	SelectObject(g_hMemDC, g_hMemBitmapOld); // DC와 해당 비트맵을 연결
	DeleteObject(g_hMemBitmap); // 비트맵 삭제
	DeleteObject(g_hMemDC); // DC 삭제

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
	case WM_LBUTTONDOWN:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		HDC hdc = GetDC(hWnd);

		TextOut(hdc, rect.right / 2, rect.bottom / 2, L"안녕하세요", 5);
		break;
	}
	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		RECT Rect;

		SetTimer(hWnd, 1, 50, nullptr);

		GetClientRect(hWnd, &Rect); // 현 윈도우와 같은 사이즈를 얻어옴
		g_hMemDC = CreateCompatibleDC(hdc); // 현재 윈도우에서 사용하고 있는 색상과 같은 걸로 만들어짐 비트맵에 접근하위한 DC임
		g_hMemBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom); // 받아온 크기와 같은 크기의 bitMap 생성
		g_hMemBitmapOld = (HBITMAP)SelectObject(g_hMemDC, g_hMemBitmap); // 아까 생성성 DC와 비트맵을 연결합니다.
		ReleaseDC(hWnd, hdc);

		PatBlt(g_hMemDC, 0, 0, Rect.right, Rect.bottom, WHITENESS); // 해당 비트맵을 하얀색 브러쉬로 칠합니다.

		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 1:
		{
			RECT rect;
			GetClientRect(hWnd, &rect); // 윈도우 창의 크기를 얻음

			int y = rect.bottom / 2 + (rand() % rect.bottom) / 2 - rect.bottom / 4; // 높이를 랜덤으로 만들고

			enqueue(y);
			InvalidateRect(hWnd, nullptr, false); // WM_PAINT를 호출하되 잔상을 지우지 않음 -> 깜박임이 없어짐
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
		RECT rect;
		GetClientRect(hWnd, &rect);
		PatBlt(g_hMemDC, 0, 0, rect.right, rect.bottom, BLACKNESS); // 비트맵을 하얗게 브러쉬로 칠함

		int y = peakqueue(queueFront);
		int count = 0;
		SelectObject(g_hMemDC, gPen); // 펜 지정
		MoveToEx(g_hMemDC, count, y, nullptr); // 비트맵 DC 에 그리기 시작
		for (int i = queueFront; i != queueRear; i = (i + 1) % QUEUE_SIZE)
		{
			count += 10;
			y = peakqueue(i);
			LineTo(g_hMemDC, count, y);
		}

		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

		BitBlt(hdc, 0, 0, rect.right, rect.bottom, g_hMemDC, 0, 0, SRCCOPY); // 완성된 비트맵을 그대로 복사합니다. 이는 SRCCOPY 라는 옵션임
		// 다른 옵션을 선택하면 다른 결과가 나오니 주의
		// SRCCOPY : Copies the source rectangle directly to the destination rectangle.

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
