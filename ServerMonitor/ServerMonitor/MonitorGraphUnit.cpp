#include "MonitorGraphUnit.h"

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
	: mhInstance(hInstance)
	, mhWndParent(hWndParent)
	, menType(enType)
	, miPosX(iPosX)
	, miPosY(iPosY)
	, miWidth(iWidth)
	, miHeight(iHeight)
{
	_hMemDC = GetDC(hWndParent);
	_hBitmap = CreateCompatibleBitmap(_hMemDC, miWidth, miHeight);
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
	return 0;
}

BOOL MonitorGraphUnit::PutThis(void)
{
	return 0;
}

MonitorGraphUnit* MonitorGraphUnit::GetThis(HWND hWnd)
{
	return nullptr;
}
