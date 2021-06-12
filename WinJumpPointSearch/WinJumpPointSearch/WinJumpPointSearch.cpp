// WinRedBlackTree.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WinJumpPointSearch.h"
#include <windowsx.h>
#include <locale>
#include "JumpPoint.h"

// 전역 변수:
HWND gMainWindow;
HINSTANCE gInstance;
HBRUSH g_White;
HBRUSH g_Gray;
HBRUSH g_Yellow;
HBRUSH g_Red; // 도착
HBRUSH g_Green; // 시작
HBRUSH g_Blue;
HFONT g_font;
HPEN g_arrow;
TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
Coordi g_begin = { MAP_WIDTH / 5, MAP_HEIGHT / 2 };
Coordi g_end = { MAP_WIDTH / 5 * 2, MAP_HEIGHT / 2 };

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool CreateMainWindow(HINSTANCE hInstance, LPCWSTR className, LPCWSTR windowName);
void OpenConsole();
void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
void DrawBegin(int x, int y, HDC hdc);
void DrawEnd(int x, int y, HDC hdc);
void DrawPoint(int x, int y, HBRUSH brush, HDC hdc);
void DrawWall(HDC hdc);
void Clear(HDC hdc);
void ClearWall(HDC hdc);


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
    static HWND btn_insert;
    static HWND btn_delete;
    static bool s_LButton = false;
    static bool s_BeginButton = false;
    static bool s_EndButton = false;
    static bool s_EraseWall = false;

    switch (message)
    {
    case WM_CREATE:
    {
        int x = GetSystemMetrics(SM_CXSCREEN) * 0.1;
        int y = GetSystemMetrics(SM_CYSCREEN) * 0.8;

        CreateWindowW(L"button", L"시작", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y + 25, 100, 20, hWnd, (HMENU)1, gInstance, NULL);
        CreateWindowW(L"button", L"클리어", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 240, y + 25, 100, 20, hWnd, (HMENU)2, gInstance, NULL);
        CreateWindowW(L"button", L"장벽 클리어", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y + 50, 100, 20, hWnd, (HMENU)3, gInstance, NULL);
        CreateWindowW(L"button", L"시작점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y - 75, 100, 20, hWnd, (HMENU)4, gInstance, NULL);
        CreateWindowW(L"button", L"도착점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 240, y - 75, 100, 20, hWnd, (HMENU)5, gInstance, NULL);
        CreateWindowW(L"button", L"장벽", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 120, y - 50, 100, 20, hWnd, (HMENU)6, gInstance, NULL);
        CreateWindowW(L"button", L"지우개", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x + 240, y - 50, 100, 20, hWnd, (HMENU)7, gInstance, NULL);

        g_White = CreateSolidBrush(RGB(200, 200, 200));
        g_Gray = CreateSolidBrush(RGB(30, 30, 30));
        g_Yellow = CreateSolidBrush(RGB(180, 180, 100));
        g_Red = CreateSolidBrush(RGB(200, 0, 0));
        g_Green = CreateSolidBrush(RGB(0, 200, 0));
        g_Blue = CreateSolidBrush(RGB(100, 100, 180));
        g_arrow = CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
        g_font = CreateFont(FONT_HEIGHT, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
            VARIABLE_PITCH | FF_ROMAN, NULL);

        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            SetFocus(gMainWindow);
            break;
        }
    }
    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam) / CELL_SIZE;
        int y = HIWORD(lParam) / CELL_SIZE;
        HDC hdc = GetDC(hWnd);

        if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT)
        {
            break;
        }

        if (!s_LButton)
            break;

        if (s_BeginButton)
        {
            DrawBegin(x, y, hdc);
        }
        else if (s_EndButton)
        {
            DrawEnd(x, y, hdc);
        }
        else if (s_EraseWall)
        {
            g_Map[y][x] = TileType::TILE_TYPE_PATH;

            DrawCell(x, y, g_White, hdc);
        }
        else
        {
            g_Map[y][x] = TileType::TILE_TYPE_WALL;

            DrawCell(x, y, g_Gray, hdc);
        }

        break;
    }
    case WM_LBUTTONDOWN:
    {
        s_LButton = true;
        break;
    }
    case WM_LBUTTONUP:
    {
        s_LButton = false;
        break;
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case 1: // 길 탐색 시작
        {
            HDC hdc = GetDC(hWnd);
            // SearchDestination(g_begin, g_end, hdc);
            break;
        }
        case 2: // 장벽 남기고 클리어
        {
            HDC hdc = GetDC(hWnd);
            Clear(hdc);
            DrawWall(hdc);

            break;
        }
        case 3: // 장벽 클리어
        {
            HDC hdc = GetDC(hWnd);
            ClearWall(hdc);

            break;
        }
        case 4:
        {
            s_BeginButton = true;
            s_EndButton = false;
            break;
        }
        case 5:
        {
            s_BeginButton = false;
            s_EndButton = true;
            break;
        }
        case 6:
        {
            s_BeginButton = false;
            s_EndButton = false;
            s_EraseWall = false;
            break;
        }
        case 7:
        {
            s_BeginButton = false;
            s_EndButton = false;
            s_EraseWall = true;
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
        Clear(hdc);

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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINJUMPPOINTSEARCH));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(RGB(0, 100, 100));
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDI_WINJUMPPOINTSEARCH);
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
    Rectangle(hdc, x * CELL_SIZE, y * CELL_SIZE,
        (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE);
}

void DrawBegin(int x, int y, HDC hdc)
{
    DrawCell(g_begin.x, g_begin.y, g_White, hdc);
    DrawPoint(x, y, g_Green, hdc);

    g_Map[y][x] = TileType::TILE_TYPE_BEGIN;
    g_Map[g_begin.y][g_begin.x] = TileType::TILE_TYPE_PATH;

    g_begin.x = x;
    g_begin.y = y;
}

void DrawEnd(int x, int y, HDC hdc)
{
    DrawCell(g_end.x, g_end.y, g_White, hdc);
    DrawPoint(x, y, g_Red, hdc);
    g_Map[y][x] = TileType::TILE_TYPE_END;
    g_Map[g_end.y][g_end.x] = TileType::TILE_TYPE_PATH;

    g_end.x = x;
    g_end.y = y;
}

void DrawPoint(int x, int y, HBRUSH brush, HDC hdc)
{
    SelectObject(hdc, brush);
    Ellipse(hdc, x * CELL_SIZE + 2, y * CELL_SIZE + 2,
        (x + 1) * CELL_SIZE - 2, (y + 1) * CELL_SIZE - 2);
}

void DrawWall(HDC hdc)
{
    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        for (int j = 0; j < MAP_WIDTH; ++j)
        {
            if (g_Map[i][j] == TileType::TILE_TYPE_WALL)
            {
                DrawCell(j, i, g_Gray, hdc);
            }
        }
    }
}

void Clear(HDC hdc)
{
    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        for (int j = 0; j < MAP_WIDTH; ++j)
        {
            DrawCell(j, i, g_White, hdc);
        }
    }

    DrawPoint(g_begin.x, g_begin.y, g_Green, hdc);
    DrawPoint(g_end.x, g_end.y, g_Red, hdc);
}

void ClearWall(HDC hdc)
{
    for (int i = 0; i < MAP_HEIGHT; ++i)
    {
        for (int j = 0; j < MAP_WIDTH; ++j)
        {
            if (g_Map[i][j] == TileType::TILE_TYPE_WALL)
            {
                DrawCell(j, i, g_White, hdc);
                g_Map[i][j] = TileType::TILE_TYPE_PATH;
            }
        }
    }
}
