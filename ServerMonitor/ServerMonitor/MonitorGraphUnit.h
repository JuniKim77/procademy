#define dfMAXCHILD (100)
#include <Windows.h>

class MyQueue;

class MonitorGraphUnit
{
public:
	enum TYPE
	{
		LINE_SINGLE,
		LINE_MULTI,
		NUMBER,
		ONOFF
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
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
		LPARAM lParam);
	/////////////////////////////////////////////////////////
	// ������ �ֱ�.
	/////////////////////////////////////////////////////////
	BOOL InsertData(int iData);
	static void RegisterChildClass(HINSTANCE hInstance);

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
	HINSTANCE mhInstance;
	HWND mChildWnd;
	TYPE menType;
	int miPosX;
	int miPosY;
	int miWidth;
	int miHeight;

	//------------------------------------------------------
	// ���� ���۸��� �޸� DC, �޸� ��Ʈ��
	//------------------------------------------------------
	HDC mChildWindowHDC;
	HDC		mDoubleHDC;

	//------------------------------------------------------
	// ������
	//------------------------------------------------------
	//������ ť / ����Ʈ
	MyQueue* mQueue;

	// static �ɹ� �Լ��� ���ν������� This �����͸� ã�� ����
	// HWND + Class Ptr �� ���̺�
};