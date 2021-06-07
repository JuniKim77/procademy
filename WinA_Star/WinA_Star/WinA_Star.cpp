// WinRedBlackTree.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WinA_Star.h"
#include <windowsx.h>
#include <locale>

#define CELL_SIZE (16)

// 전역 변수:
HWND gMainWindow;
HINSTANCE gInstance;
HBRUSH g_White;
HBRUSH g_Gray;
HBRUSH g_Yellow;
HPEN g_Pen;
HPEN g_ArrowPen;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);
void OpenConsole();
void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
void Clear(HDC hdc);


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

    if (CreateMainWindow(hInstance, L"MainWindow", L"Binary Tree") == false)
        return 1;

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    FreeConsole();

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
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND editor;
    static HWND btn_insert;
    static HWND btn_delete;
    static bool s_button = false;

    switch (message)
    {
    case WM_CREATE:
    {
        int x = GetSystemMetrics(SM_CXSCREEN) * 0.1;
        int y = GetSystemMetrics(SM_CYSCREEN) * 0.8;

        editor = CreateWindowW(L"edit", nullptr, WS_CHILD | WS_VISIBLE | ES_NUMBER, x, y + 25, 100, 20, hWnd, (HMENU)0, gInstance, NULL);
        SendMessageW(editor, EM_LIMITTEXT, (WPARAM)10, 0);
        CreateWindowW(L"button", L"시작점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y + 25, 80, 20, hWnd, (HMENU)1, gInstance, NULL);
        CreateWindowW(L"button", L"도착점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 240, y + 25, 80, 20, hWnd, (HMENU)2, gInstance, NULL);
        CreateWindowW(L"button", L"장벽", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y + 50, 80, 20, hWnd, (HMENU)3, gInstance, NULL);
        CreateWindowW(L"button", L"클리어", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 240, y + 50, 80, 20, hWnd, (HMENU)4, gInstance, NULL);

        g_White = CreateSolidBrush(RGB(200, 200, 200));
        g_Gray = CreateSolidBrush(RGB(30, 30, 30));
        g_Yellow = CreateSolidBrush(RGB(180, 180, 100));
        g_Pen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
        g_ArrowPen = CreatePen(PS_SOLID, 1, RGB(150, 0, 0));

        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_SPACE:
            SetFocus(editor);
            break;
        case VK_ESCAPE:
            SetFocus(gMainWindow);
            break;
        }
    }
    case WM_MOUSEMOVE:
    {
        if (!s_button)
            break;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        wprintf_s(L"X: %d, Y: %d \n", x, y);

        if (x < CELL_SIZE || y < CELL_SIZE || x >= 101 * CELL_SIZE || y >= 51 * CELL_SIZE)
        {
            break;
        }
        HDC hdc = GetDC(hWnd);
        DrawCell(x / CELL_SIZE, y / CELL_SIZE, g_Yellow, hdc);
        break;
    }
    case WM_LBUTTONDOWN:
    {
        s_button = true;
        break;
    }
    case WM_LBUTTONUP:
    {
        s_button = false;
        break;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case 1:
        {
            break;
        }
        case 2:
        {
            WCHAR text[20] = { 0, };
            SendMessage(editor, WM_GETTEXT, 20, (LPARAM)text);
            SetWindowText(editor, L"");
            SetFocus(gMainWindow);

            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        }
        case 3:
        {
            SetWindowText(editor, L"");
            SetFocus(gMainWindow);

            

            InvalidateRect(hWnd, nullptr, TRUE);
            break;
        }
        case 4:
        {
            HDC hdc = GetDC(hWnd);
            Clear(hdc);

            break;
        }
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
        SelectObject(hdc, g_Pen);

        for (int i = 1; i <= 50; ++i)
        {
            for (int j = 1; j <= 100; ++j)
            {
                DrawCell(j, i, g_White, hdc);
            }
        }

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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINASTAR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 100, 100));
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_WINASTAR);
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // 애플리케이션 초기화를 수행합니다:
    HWND hWnd = CreateWindowExW(0, className, windowName, WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE,
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

void DrawCell(int x, int y, HBRUSH brush, HDC hdc)
{
    SelectObject(hdc, brush);
    Rectangle(hdc, x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE);
}

void Clear(HDC hdc)
{
    SelectObject(hdc, g_Pen);

    for (int i = 1; i <= 50; ++i)
    {
        for (int j = 1; j <= 100; ++j)
        {
            DrawCell(j, i, g_White, hdc);
        }
    }
}
