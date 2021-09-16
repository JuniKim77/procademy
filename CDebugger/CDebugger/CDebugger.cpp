#include "CDebugger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <strsafe.h>
#include <locale.h>
#include <direct.h>


WORD CDebugger::sIndex = USHRT_MAX;
WCHAR CDebugger::sFolderPath[MAX_PATH];
WCHAR* CDebugger::sLogData[USHRT_MAX + 1];
bool CDebugger::sIsOver = false;

void CDebugger::_Log(const WCHAR* format, ...)
{
    USHORT index = InterlockedIncrement16((short*)&sIndex);
    tm t;
    time_t curTime;
    int lenval = 0;
    int count = 0;
    va_list ap;


    time(&curTime);
    localtime_s(&t, &curTime);
    WCHAR* pLog = sLogData[index];

    lenval = wsprintf(pLog, L"%05d ## %02d/%02d/%02d %02d:%02d:%02d ## "
        , GetCurrentThreadId(), t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
        t.tm_hour, t.tm_min, t.tm_sec);

    pLog += lenval;
    count += lenval;

    va_start(ap, format);
    {
        DWORD len = DEBUG_CONST_MAX_LEN > count ? DEBUG_CONST_MAX_LEN - count : 0;
        StringCchVPrintf(pLog, len, format, ap);
        pLog += len;
    }
    va_end(ap);

    if (index == USHRT_MAX)
    {
        sIsOver = true;
    }
}

void CDebugger::Initialize()
{
    setlocale(LC_ALL, "");

    for (int i = 0; i <= USHRT_MAX; ++i)
    {
        sLogData[i] = new WCHAR[DEBUG_CONST_MAX_LEN];
    }
}

void CDebugger::Destroy()
{
    for (int i = 0; i < USHRT_MAX; ++i)
    {
        delete[] sLogData[i];
    }
}

void CDebugger::SetDirectory(const WCHAR* path)
{
    int iReulst = _wmkdir(path);

    ZeroMemory(sFolderPath, MAX_PATH);
    StringCchPrintf(sFolderPath, MAX_PATH, path);
}

void CDebugger::PrintLogOut(const WCHAR* szFileName)
{
    tm t;
    time_t curTime;
    WCHAR fileName[MAX_PATH];

    time(&curTime);
    localtime_s(&t, &curTime);

    wsprintf(fileName, L"%s/%04d%02d_Log.txt",
        sFolderPath, t.tm_year + 1900, t.tm_mon + 1);

    FILE* fout;

    do
    {
        _wfopen_s(&fout, fileName, L"a+");
    } while (fout == nullptr);

    for (WORD i = 0; i < sIndex; ++i)
    {
        fwprintf_s(fout, L"%s\n\n", sLogData[i]);
    }

    fwprintf_s(fout, L"\n===========================================\n\n");

    fclose(fout);
}
