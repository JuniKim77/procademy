#pragma once
#include "CCrashDump.h"
#include "CDebugger.h"

long procademy::CCrashDump::_DumpCount = 0;

procademy::CCrashDump::CCrashDump()
{
    _DumpCount = 0;

    _invalid_parameter_handler oldHandler, newHandler;
    newHandler = myInvalidParameterHandler;

    oldHandler = _set_invalid_parameter_handler(newHandler);   // crt�Լ��� null ������ ���� �־��� ��
    _CrtSetReportMode(_CRT_WARN, 0);                     // CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������.
    _CrtSetReportMode(_CRT_ASSERT, 0);                     // CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������.
    _CrtSetReportMode(_CRT_ERROR, 0);                     // CRT ���� �޽��� ǥ�� �ߴ�. �ٷ� ������ ������.

    _CrtSetReportHook(_custom_Report_hook);

    //-----------------------------------------------------------------------
    // pure virtual function called ���� �ڵ鷯�� ����� ���� �Լ��� ��ȸ��Ų��.
    //-----------------------------------------------------------------------
    _set_purecall_handler(myPurecallHandler);

    SetHandlerDump();
};

void procademy::CCrashDump::Crash(void)
{
    *((int*)(0x00000000)) = 0;
}

LONG WINAPI procademy::CCrashDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
    // CDebugger::PrintLogOut(L"debugs.txt");

    int iResult = _mkdir("../Dump");

    int iWorkingMemory = 0;
    SYSTEMTIME stNowTime;

    long DumpCount = InterlockedIncrement(&_DumpCount);

    //----------------------------------------------------------
    // ���� ���μ����� �޸� ��뷮�� ���´�.
    //----------------------------------------------------------
    HANDLE hProcess = 0;
    PROCESS_MEMORY_COUNTERS pmc;

    hProcess = GetCurrentProcess();

    if (NULL == hProcess)
        return 0;

    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
    }
    CloseHandle(hProcess);

    //----------------------------------------------------------
    // ���� ��¥�� �ð��� �˾ƿ´�.
    //----------------------------------------------------------
    WCHAR filename[MAX_PATH];

    GetLocalTime(&stNowTime);
    wsprintf(
        filename,
        L"../Dump/Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp"
        , stNowTime.wYear
        , stNowTime.wMonth
        , stNowTime.wDay
        , stNowTime.wHour
        , stNowTime.wMinute
        , stNowTime.wSecond
        , DumpCount
        , iWorkingMemory
    );
    wprintf(L"\n\n\n!!! Crash Error!!! %d.%d.%d / %d:%d:%d \n"
        , stNowTime.wYear
        , stNowTime.wMonth
        , stNowTime.wDay
        , stNowTime.wHour
        , stNowTime.wMinute
        , stNowTime.wSecond
    );
    wprintf(L"Now Save dump file...\n");

    HANDLE hDumpFile = ::CreateFileW(
        filename,
        GENERIC_WRITE,            // ���μ��� ������ ������ ���� ���� ����.
        FILE_SHARE_WRITE,         // �ٸ� ���μ������� �� ���Ͽ� ���� ���� ���� ����
        NULL,
        CREATE_ALWAYS,            // �׻� ���ο� ���� ����
        FILE_ATTRIBUTE_NORMAL,      // ���� ���Ϸ� ����
        NULL
    );

    if (hDumpFile != INVALID_HANDLE_VALUE)
    {
        _MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

        MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
        MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
        MinidumpExceptionInformation.ClientPointers = TRUE;

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpWithFullMemory, &MinidumpExceptionInformation, NULL, NULL);
        CloseHandle(hDumpFile);
        wprintf(L"CrashDump Svae Finish!\n");
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void procademy::CCrashDump::SetHandlerDump(void)
{
    SetUnhandledExceptionFilter(MyExceptionFilter);
}

void procademy::CCrashDump::myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int   line, uintptr_t   pReserved)
{
    Crash();
}

int procademy::CCrashDump::_custom_Report_hook(int ireposttype, char* message, int* returnvalue)
{
    Crash();
    return true;
}

void procademy::CCrashDump::myPurecallHandler(void)
{
    Crash();
}