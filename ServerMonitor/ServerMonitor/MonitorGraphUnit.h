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
	// ������ ���ν���
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT meesage, WPARAM wParam,
		LPARAM lParam);
	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	BOOL InsertData(int iData);

protected:
	//------------------------------------------------------
	// ������ �ڵ�, this ������ ��Ī ���̺� ����.
	//------------------------------------------------------
	BOOL PutThis(void);
	static MonitorGraphUnit* GetThis(HWND hWnd);

private:
	//------------------------------------------------------
	// �θ� ������ �ڵ�, �� ������ �ڵ�, �ν��Ͻ� �ڵ�
	//------------------------------------------------------
	//------------------------------------------------------
	// ������ ��ġ,ũ��,����, �׷��� Ÿ�� ��.. �ڷ�
	//------------------------------------------------------
	TYPE _enGraphType;

	//------------------------------------------------------
	// ���� ���۸��� �޸� DC, �޸� ��Ʈ��
	//------------------------------------------------------
	HDC		_hMemDC;
	HBITMAP		_hBitmap;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------
	//������ ť / ����Ʈ

	// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
	// HWND + Class Ptr �� ���̺�
};