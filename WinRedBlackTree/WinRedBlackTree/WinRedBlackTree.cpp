// WinRedBlackTree.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WinRedBlackTree.h"
#include <windowsx.h>
#include <locale>

// 전역 변수:
HWND gMainWindow;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);
void OpenConsole();


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.

    // 애플리케이션 초기화를 수행합니다:
    OpenConsole();

    if (CreateMainWindow(hInstance, L"MainWindow", L"Red Black Tree") == false)
        return 1;

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
    }

    FreeConsole();

    return (int) msg.wParam;
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
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        HDC hdc = GetDC(hWnd);

        int x = rand() % 1000;
        int y = rand() % 1000;

        TextOutW(hdc, x, y, L"A", 1);

        ReleaseDC(hWnd, hdc);
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
            int x = GetSystemMetrics(SM_CXSCREEN) * 0.1;
            int y = GetSystemMetrics(SM_CYSCREEN) * 0.8;
            WCHAR str[] = L"1. 노드 입력    |    2. 노드 삭제";
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, x, y, str, _countof(str) - 1);
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

bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINREDBLACKTREE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_WINREDBLACKTREE);
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // 애플리케이션 초기화를 수행합니다:
    HWND hWnd = CreateWindowExW(0, className, windowName, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, hInstance, nullptr);

    if (hWnd == nullptr)
        return false;

    gMainWindow = hWnd;

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    SetFocus(hWnd);

    return true;
}

void OpenConsole()
{
    setlocale(LC_ALL, "");

    FILE* fin;
    FILE* fout;
    FILE* ferr;

    if (AllocConsole())
    {
        freopen_s(&fin, "CONIN$", "r", stdin);
        freopen_s(&ferr, "CONOUT$", "w", stderr);
        freopen_s(&fout, "CONOUT$", "w", stdout);
    }

    system("mode con: cols=80 lines=20");
}
