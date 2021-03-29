// ScreenDIB.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "ScreenDIB.h"
#include <stdio.h>
#include "CDib.h"

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
CDib gDib(256, 256, 32, true);
BITMAPFILEHEADER fileHeader;
BITMAPINFOHEADER infoHeader;
BYTE* imageBuffer;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void LoadBMPFile(const char* fileName);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENDIB));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SCREENDIB);
    wcex.lpszClassName = L"ABC";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // 애플리케이션 초기화를 수행합니다:
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(L"ABC", L"타이틀", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    LoadBMPFile("sample.bmp");

    InvalidateRect(hWnd, nullptr, false);

    // 기본 메시지 루프입니다:
    while (1)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            BYTE* buffer = gDib.GetDibBuffer();
            BYTE color = 0;

            for (int i = 0; i < gDib.GetHeight(); ++i)
            {
                memset(buffer, color, gDib.GetPitch());
                buffer += gDib.GetPitch();
                color++;
            }

            DWORD red = 0x00ff0000;
            buffer = gDib.GetDibBuffer();

            for (int i = 0; i < gDib.GetHeight(); ++i)
            {
                *(DWORD*)(buffer + i * 4) = red;
                buffer += gDib.GetPitch();
            }

            gDib.Filp(hWnd, 500, 500);
        }
    }

    free(imageBuffer);

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
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
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
            CDib dib(infoHeader.biWidth, infoHeader.biHeight, infoHeader.biBitCount);
            BYTE* buffer = dib.GetDibBuffer();
            BYTE* pImage = imageBuffer;
            int pitch = dib.GetPitch();

            for (int i = 0; i < dib.GetHeight(); ++i)
            {
                memcpy_s(buffer, pitch, pImage, pitch);
                buffer += pitch;
                pImage += pitch;
            }

            dib.Filp(hWnd);

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

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void LoadBMPFile(const char* fileName)
{
    FILE* fin;

    fopen_s(&fin, fileName, "rb");
    fseek(fin, 0, SEEK_END);

    int size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    BYTE* buffer = (BYTE*)malloc(size);
    BYTE* pBuf = buffer;

    fread_s(buffer, size, size, 1, fin);

    memcpy_s(&fileHeader, sizeof(fileHeader), pBuf, sizeof(fileHeader));
    pBuf += sizeof(fileHeader);
    memcpy_s(&infoHeader, sizeof(infoHeader), pBuf, sizeof(infoHeader));
    pBuf += sizeof(infoHeader);
    
    int pitch = (infoHeader.biWidth * infoHeader.biBitCount / 8 + 3) & ~3;

    imageBuffer = (BYTE*)malloc(pitch * infoHeader.biHeight);
    BYTE* pImage = imageBuffer;

    for (int i = 0; i < infoHeader.biHeight; ++i)
    {
        memcpy_s(pImage, pitch, pBuf, pitch);
        pBuf += pitch;
        pImage += pitch;
    }

    free(buffer);

}
