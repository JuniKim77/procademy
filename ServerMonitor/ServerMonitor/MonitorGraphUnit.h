#define dfMAXCHILD (100)
#include <Windows.h>

class MonitorGraphUnit
{
public:
	enum TYPE
	{
		LINE_SINGLE,
		LINE_MULTI,
		NUMBER,
		ONOFF,
		PIE
	};
	struct ST_HWNDtoTHIS
	{
		HWND hWnd[dfMAXCHILD];
		MonitorGraphUnit* pThis[dfMAXCHILD];
	};

public:
	MonitorGraphUnit(HINSTANCE hInstance, HWND hWndParent, TYPE enType, int iPosX,
		int iPosY, int iWidth, int iHeight);
	~MonitorGraphUnit();
	/////////////////////////////////////////////////////////
	// 윈도우 프로시저
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT meesage, WPARAM wParam,
		LPARAM lParam);
	/////////////////////////////////////////////////////////
	// 데이터 넣기.
	/////////////////////////////////////////////////////////
	BOOL InsertData(int iData);

protected:
	//------------------------------------------------------
	// 윈도우 핸들, this 포인터 매칭 테이블 관리.
	//------------------------------------------------------
	BOOL PutThis(void);
	static MonitorGraphUnit* GetThis(HWND hWnd);

private:
	//------------------------------------------------------
	// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
	//------------------------------------------------------
	//------------------------------------------------------
	// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
	//------------------------------------------------------
	TYPE _enGraphType;

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// 데이터
	//------------------------------------------------------
	//데이터 큐 / 리스트

	// static 맴버 함수의 프로시저에서 This 포인터를 찾기 위한
	// HWND + Class Ptr 의 테이블
};