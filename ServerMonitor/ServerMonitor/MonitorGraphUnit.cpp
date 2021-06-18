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
	HWND childWnd = CreateWindow(L"child_window", NULL, WS_CHILD | WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS,
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

LRESULT MonitorGraphUnit::WndProc(HWND hWnd, UINT meesage, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
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
