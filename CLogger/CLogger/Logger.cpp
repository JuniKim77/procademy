#include "Logger.h"

//////////////////////////////////////////////////////////////////////////////////

DWORD         g_LogCount;
en_LOG_LEVEL   g_LogLevel;

FILE* g_File;
SRWLOCK         g_File_srw;

WCHAR         g_szBuffer[MAX_PATH];
SRWLOCK         g_szBuffer_srw;

//////////////////////////////////////////////////////////////////////////////////

void InitializeLogger(void)
{
    setlocale(LC_ALL, "");
    InitializeSRWLock(&g_File_srw);
    InitializeSRWLock(&g_szBuffer_srw);
}

void SetDirectory(const WCHAR* szFolderName)
{
    LockFoler();

    //----------------------------------------------
    // ���� ����
    //----------------------------------------------
    int iReulst = _wmkdir(szFolderName);

    //----------------------------------------------
    // ���� ��� ����
    //----------------------------------------------      
    ZeroMemory(g_szBuffer, MAX_PATH);
    StringCchPrintf(g_szBuffer, MAX_PATH, szFolderName);

    UnlockFoler();
}

//////////////////////////////////////////////////////////////////////////////////

void Log(const WCHAR* szType, en_LOG_LEVEL LogLevel, const WCHAR* szStringFormat, ...)
{
    //----------------------------------------------
    // �α� ī���� ����
    //----------------------------------------------
    DWORD dwLogCount = InterlockedIncrement(&g_LogCount);

    //----------------------------------------------
    // �������� ��� ���ڿ� ����
    //----------------------------------------------
    wchar_t szInMessage[MAX_PATH];
    va_list va;
    va_start(va, szStringFormat);
    StringCchVPrintf(szInMessage, MAX_PATH, szStringFormat, va);
    va_end(va);

    //----------------------------------------------
    // �α� Ÿ�� ���ڿ� ����
    //----------------------------------------------
    wchar_t szLogLevel[32];
    switch (LogLevel)
    {
    case en_LOG_LEVEL::LEVEL_DEBUG:
        wsprintf(szLogLevel, L"%s", L"DEBUG");
        break;
    case en_LOG_LEVEL::LEVEL_ERROR:
        wsprintf(szLogLevel, L"%s", L"ERROR");
        break;
    case en_LOG_LEVEL::LEVEL_SYSTEM:
        wsprintf(szLogLevel, L"%s", L"SYSTEM");
        break;
    default:
        break;
    }

    //----------------------------------------------
    // ���� ��¥�� �ð��� �˾ƿ´�.
    //----------------------------------------------
    wchar_t szFileName[MAX_PATH];
    SYSTEMTIME stNowTime;
    GetLocalTime(&stNowTime);
    wsprintf(szFileName,
        L"%s/%d%02d%_%s.txt",
        g_szBuffer,
        stNowTime.wYear,
        stNowTime.wMonth,
        szType
    );

    //----------------------------------------------
    // ���Ͽ� ����
    //----------------------------------------------
    LockFlie();
    do
    {
        _wfopen_s(&g_File, szFileName, L"a+");
    } while (g_File == nullptr);

    fwprintf_s(g_File,
        L"[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09d] %s\n",
        szType,
        stNowTime.wYear,
        stNowTime.wMonth,
        stNowTime.wDay,
        stNowTime.wHour,
        stNowTime.wMinute,
        stNowTime.wSecond,
        szLogLevel,
        dwLogCount,
        szInMessage
    );
    fclose(g_File);
    UnlockFlie();
}

//////////////////////////////////////////////////////////////////////////////////

void LogHex(const WCHAR* szType, en_LOG_LEVEL LogLevel, const WCHAR* szLog, char* pByte, int iByteLen, int iRow)
{
    //----------------------------------------------
    // �α� ī���� ����
    //----------------------------------------------
    DWORD dwLogCount = InterlockedIncrement(&g_LogCount);

    //----------------------------------------------
    // �α� Ÿ�� ���ڿ� ����
    //----------------------------------------------
    wchar_t szLogLevel[32];
    switch (LogLevel)
    {
    case en_LOG_LEVEL::LEVEL_DEBUG:
        wsprintf(szLogLevel, L"%s", L"DEBUG");
        break;
    case en_LOG_LEVEL::LEVEL_ERROR:
        wsprintf(szLogLevel, L"%s", L"ERROR");
        break;
    case en_LOG_LEVEL::LEVEL_SYSTEM:
        wsprintf(szLogLevel, L"%s", L"SYSTEM");
        break;
    default:
        break;
    }

    //----------------------------------------------
    // ���� ��¥�� �ð��� �˾ƿ´�.
    //----------------------------------------------
    wchar_t szFileName[MAX_PATH];
    SYSTEMTIME stNowTime;
    GetLocalTime(&stNowTime);
    wsprintf(szFileName,
        L"%s/%d%02d%_%s.txt",
        g_szBuffer,
        stNowTime.wYear,
        stNowTime.wMonth,
        szType
    );

    //----------------------------------------------
    // ���Ͽ� ����
    //----------------------------------------------
    LockFlie();
    do
    {
        _wfopen_s(&g_File, szFileName, L"a");
    } while (g_File == nullptr);

    fwprintf_s(g_File,
        L"[%s] [%d-%02d-%02d %02d:%02d:%02d / %s / %09d] %s\n",
        szType,
        stNowTime.wYear,
        stNowTime.wMonth,
        stNowTime.wDay,
        stNowTime.wHour,
        stNowTime.wMinute,
        stNowTime.wSecond,
        szLogLevel,
        dwLogCount,
        szLog
    );

    //----------------------------------------------
    // little endian
    //----------------------------------------------
    for (int iCount = 0; iCount < iByteLen; iCount++)
    {
        fwprintf_s(g_File, L"%02x ", pByte[iCount] & 0xff);
        if ((iCount + 1) % iRow == 0)
        {
            fwprintf_s(g_File, L"\n");
        }
    }

    //----------------------------------------------
    // big Endian
    //----------------------------------------------
    //for (int iCount = iByteLen - 1; -1 < iCount; iCount--)
    //{
    //   fwprintf_s(g_LoggerFile, L"%02x ", pByte[iCount]);
    //}

    fwprintf_s(g_File, L"\n");
    fclose(g_File);

    UnlockFlie();
}

//////////////////////////////////////////////////////////////////////////////////

void LogConsole(en_LOG_LEVEL LogLevel, const WCHAR* szStringFormat, ...)
{
    //----------------------------------------------
    // ���õǾ��ִ� �α� ������ �ƴϸ� ����
    //----------------------------------------------
    if (g_LogLevel != LogLevel)
    {
        return;
    }

    //----------------------------------------------
    // �������� ��� ���ڿ� ����
    //----------------------------------------------
    wchar_t szInMessage[MAX_PATH];
    va_list va;
    va_start(va, szStringFormat);
    StringCchVPrintf(szInMessage, MAX_PATH, szStringFormat, va);
    va_end(va);

    wprintf_s(L"%s", szInMessage);
}