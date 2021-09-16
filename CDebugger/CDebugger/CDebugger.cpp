#include "CDebugger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <strsafe.h>
#include <locale.h>
#include <direct.h>


WORD CDebugger::sIndex = USHRT_MAX;
WCHAR CDebugger::sFolderPath[MAX_PATH];
WCHAR* CDebugger::sLogData[USHRT_MAX];

void CDebugger::_Log(const WCHAR* format, ...)
{
    USHORT index = InterlockedIncrement16((short*)&sIndex);
    int lenval = 0;
    int count = 0;
    WCHAR* pLog = sLogData[index];

    lenval = swprintf_s(pLog, )


    printf("%u\n", GetCurrentThreadId());
}

void CDebugger::Initialize(DWORD size)
{
    setlocale(LC_ALL, "");

    for (int i = 0; i < USHRT_MAX; ++i)
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
