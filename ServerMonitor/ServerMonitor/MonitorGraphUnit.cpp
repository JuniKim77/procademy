#include "MonitorGraphUnit.h"
#include "MyQueue.h"
#include <stdio.h>

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
	: mhInstance(hInstance)
	, menType(enType)
	, miPosX(iPosX)
	, miPosY(iPosY)
	, miWidth(iWidth)
	, miHeight(iHeight)
{
	mChildWnd = CreateWindow(L"child_window", NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		miPosX, miPosY, miWidth, miHeight, hWndParent, NULL, mhInstance, NULL);
	mChildWindowHDC = GetDC(mChildWnd);
	mDoubleHDC = CreateCompatibleDC(mChildWindowHDC);
	HBITMAP mBitMap = CreateCompatibleBitmap(mChildWindowHDC, miWidth, miHeight);
	SelectObject(mDoubleHDC, mBitMap);
	mQueue = new MyQueue(100);

	ShowWindow(mChildWnd, SW_SHOW);
	UpdateWindow(mChildWnd);
}

MonitorGraphUnit::~MonitorGraphUnit()
{
}

LRESULT MonitorGraphUnit::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:

		break;

	case WM_TIMER:
		
		break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 메뉴 선택을 구문 분석합니다:
		switch (wmId)
		{
		
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

BOOL MonitorGraphUnit::InsertData(int iData)
{
	HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	mQueue->enqueue(iData);
	
	int y = mQueue->peakqueue(mQueue->mFront);
	int count = 0;
	RECT rect;
	GetClientRect(mChildWnd, &rect);

	PatBlt(mDoubleHDC, 0, 0, rect.right, rect.bottom, WHITENESS);
	SelectObject(mDoubleHDC, redPen);
	MoveToEx(mDoubleHDC, count, 50 + y, nullptr); // 비트맵 DC 에 그리기 시작

	for (int i = mQueue->mFront; i != mQueue->mRear; i = (i + 1) % mQueue->mCapacity)
	{
		count += 3;
		y = mQueue->peakqueue(i);
		LineTo(mDoubleHDC, count, 50 + y);
	}

	BitBlt(mChildWindowHDC, 0, 0, rect.right, rect.bottom, mDoubleHDC, 0, 0, SRCCOPY);

	DeleteObject(redPen);

	return 0;
}

void MonitorGraphUnit::RegisterChildClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"child_window";
	wcex.hIconSm = NULL;

	RegisterClassExW(&wcex);
}

BOOL MonitorGraphUnit::PutThis(void)
{
	return 0;
}

MonitorGraphUnit* MonitorGraphUnit::GetThis(HWND hWnd)
{
	return nullptr;
}
