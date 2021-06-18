#include "MonitorGraphUnit.h"
#include "MyQueue.h"

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
	: mhInstance(hInstance)
	, mhWndParent(hWndParent)
	, menType(enType)
	, miPosX(iPosX)
	, miPosY(iPosY)
	, miWidth(iWidth)
	, miHeight(iHeight)
{
	HWND childWnd = CreateWindow(L"child_window", NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		miPosX, miPosY, miWidth, miHeight, mhWndParent, NULL, mhInstance, NULL);
	_hMemDC = GetDC(childWnd);
	_hBitmap = CreateCompatibleBitmap(_hMemDC, miWidth, miHeight);
	mQueue = new MyQueue(100);

	ShowWindow(childWnd, SW_SHOW);
	UpdateWindow(childWnd);
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
	mQueue->enqueue(iData);

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
