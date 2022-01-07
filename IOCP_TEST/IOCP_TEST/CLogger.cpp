#include "CLogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <strsafe.h>
#include <locale.h>
#include <windows.h>
#include <direct.h>

#define dfLOG_SIZE (1024)

int CLogger::mLogLevel = dfLOG_LEVEL_DEBUG;
DWORD CLogger::mLogCount = 0;
SRWLOCK CLogger::mSrwLock;
WCHAR CLogger::mFolderPath[MAX_PATH];

void CLogger::_Log(int logLevel, const WCHAR* format, ...)
{
    WCHAR       fileName[MAX_PATH];
    DWORD       dwLogCount;
    tm          t;
    time_t      curTime;
    int         lenval = 0;
    int         count = 0;
    WCHAR       log[dfLOG_SIZE];
    WCHAR*      pLog;
    va_list     ap;

    if (mLogLevel == dfLOG_LEVEL_OFF)
        return;
    if (logLevel == dfLOG_LEVEL_DEBUG && mLogLevel > logLevel)
        return;

    dwLogCount = InterlockedIncrement(&mLogCount);

    time(&curTime);
    localtime_s(&t, &curTime);

    pLog = log;

    switch (logLevel)
    {
    case dfLOG_LEVEL_DEBUG:
        lenval = swprintf_s(pLog, dfLOG_SIZE, L"[DEBUG] [%d] ", dwLogCount);
        break;
    case dfLOG_LEVEL_ERROR:
        lenval = swprintf_s(pLog, dfLOG_SIZE, L"[ERROR] [%d] ", dwLogCount);
        swprintf_s(fileName, _countof(fileName), L"%s/%04d%02d_Error_Log.txt",
            mFolderPath, t.tm_year + 1900, t.tm_mon + 1);
        break;
    case dfLOG_LEVEL_SYSTEM:
        lenval = swprintf_s(pLog, dfLOG_SIZE, L"[SYSTEM] [%d] ", dwLogCount);
        swprintf_s(fileName, _countof(fileName), L"%s/%04d%02d_System_Log.txt",
            mFolderPath, t.tm_year + 1900, t.tm_mon + 1);
        break;
    default:
        break;
    }

    pLog += lenval;
    count += lenval;

    lenval = swprintf_s(pLog, dfLOG_SIZE, L"[%02d/%02d/%02d %02d:%02d:%02d] ",
        t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
        t.tm_hour, t.tm_min, t.tm_sec);

    pLog += lenval;
    count += lenval;

    va_start(ap, format);
    {
        DWORD len = dfLOG_SIZE > count ? dfLOG_SIZE - count : 0;
        StringCchVPrintf(pLog, len, format, ap);
    }
    va_end(ap);

    if (logLevel >= mLogLevel)
    {
        wprintf_s(log);
    }

    if (logLevel == dfLOG_LEVEL_DEBUG)
    {
        return;
    }

    LockFile();
    {
        FILE* fout;

        do
        {
            _wfopen_s(&fout, fileName, L"a+");
        } while (fout == nullptr);

        fwprintf_s(fout, L"%s\n\n", log);

        fclose(fout);
    }
    UnlockFile();
}

void CLogger::Initialize()
{
    ZeroMemory(mFolderPath, MAX_PATH);
    setlocale(LC_ALL, "");
    InitializeSRWLock(&mSrwLock);
}

void CLogger::SetDirectory(const WCHAR* path)
{
    int iReulst = _wmkdir(path);

    ZeroMemory(mFolderPath, MAX_PATH);
    StringCchPrintf(mFolderPath, MAX_PATH, path);
}

void CLogger::LockFile()
{
    AcquireSRWLockExclusive(&mSrwLock);
}

void CLogger::UnlockFile()
{
    ReleaseSRWLockExclusive(&mSrwLock);
}
