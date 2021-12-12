#include "CMMOEchoServer.h"
#include <conio.h>
#include "CProfiler.h"
#include "TextParser.h"

procademy::CMMOEchoServer::CMMOEchoServer()
{
    LoadInitFile(L"Server.cnf");
    Init();
    BeginThreads();
}

procademy::CMMOEchoServer::~CMMOEchoServer()
{
    delete[] mPlayers;
}

void procademy::CMMOEchoServer::AllocSessions(int num)
{
    mPlayers = new CPlayer[num];

    for (int i = 0; i < num; ++i)
    {
        SetSession(&mPlayers[i]);
    }
}

bool procademy::CMMOEchoServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CMMOEchoServer::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CMMOEchoServer::BeginServer()
{
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    WaitForThreadsFin();

    // 종료 코드

    DWORD ret = WaitForSingleObject(mMonitorThread, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CMMOEchoServer Thread Handle Error");
        break;
    case WAIT_TIMEOUT:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CMMOEchoServer Thread Timeout Error");
        break;
    case WAIT_OBJECT_0:
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CMMOEchoServer End");
        break;
    default:
        break;
    }

    return true;
}

void procademy::CMMOEchoServer::WaitForThreadsFin()
{
    while (1)
    {
        char ch = _getch();

        switch (ch)
        {
        case 'g':
            mbNagle = !mbNagle;
            SetNagle(mbNagle);
            break;
        case 'z':
            mbZeroCopy = !mbZeroCopy;
            SetZeroCopy(mbZeroCopy);
            break;
        case 'm':
            if (mbMonitoring)
            {
                wprintf(L"Unset Monitoring\n");
                mbMonitoring = false;
            }
            else
            {
                wprintf(L"Set Monitoring\n");
                mbMonitoring = true;
            }
            break;
        case 's':
            if (mbBegin)
            {
                Stop();
                wprintf(L"STOP\n");
            }
            else
            {
                Start();
                wprintf(L"RUN\n");
            }
            break;
        case 'p':
            wprintf(L"Print Profiler\n");
            CProfiler::Print();
            break;
        case 'd':
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CMMOEchoServer Intended Crash");
            CRASH();
        case 'q':
            QuitServer();
            return;
        default:
            break;
        }
    }
}

unsigned int __stdcall procademy::CMMOEchoServer::MonitorThread(LPVOID arg)
{
    CMMOEchoServer* echoServer = (CMMOEchoServer*)arg;

    echoServer->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End");

    return 0;
}

void procademy::CMMOEchoServer::BeginThreads()
{
}

void procademy::CMMOEchoServer::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.LoadFile(fileName);

    tp.GetValue(L"CLIENT_MAX", &num);
    mMaxClient = (u_short)num;
}

void procademy::CMMOEchoServer::Init()
{
    AllocSessions(mMaxClient);
}

void procademy::CMMOEchoServer::MonitoringProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
    WCHAR str[2048];

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            mCpuUsage.UpdateProcessorCpuTime();
        }
    }

    CloseHandle(dummyevent);
}
