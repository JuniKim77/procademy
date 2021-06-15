#include "MonitorGraphUnit.h"

MonitorGraphUnit::MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX, int iPosY, int iWidth, int iHeight)
	: mHInstance(hInstance)
	, mHParent(hWndParent)
	, _enGraphType(enType)
	, mX(iPosX)
	, mY(iPosY)
	, mWidth(iWidth)
	, mHeight(iHeight)
{
	_hMemDC = GetDC(mHParent);
	_hBitmap = CreateBitmap(iWidth, iHeight, 1, 32, nullptr);
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
