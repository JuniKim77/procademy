#include "CMonitorServer.h"
#include "TextParser.h"
#include <conio.h>

procademy::CMonitorServer::CMonitorServer()
{
    LoadInitFile(L"Server.cnf");
    Init();
    BeginThreads();
}

procademy::CMonitorServer::~CMonitorServer()
{
    delete[] mThreads;
}

bool procademy::CMonitorServer::BeginServer()
{
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    WaitForThreadsFin();

    DWORD ret = WaitForMultipleObjects(2, mThreads, true, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CMonitorServer Thread Handle Error");
        break;
    case WAIT_TIMEOUT:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CMonitorServer Thread Timeout Error");
        break;
    case WAIT_OBJECT_0:
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CMonitorServer End");
        break;
    default:
        break;
    }

    return true;
}

bool procademy::CMonitorServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CMonitorServer::OnClientJoin(SESSION_ID SessionID)
{
}

void procademy::CMonitorServer::OnClientLeave(SESSION_ID SessionID)
{
}

void procademy::CMonitorServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
}

void procademy::CMonitorServer::OnError(int errorcode, const WCHAR* log)
{
}

void procademy::CMonitorServer::Init()
{
    InitializeSRWLock(&mServerLock);
    mThreads = new HANDLE[2];
}

bool procademy::CMonitorServer::MonitorProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
    WCHAR str[2048];

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            // Ãâ·Â
            MakeMonitorStr(str, 2048);

            wprintf(str);

            ClearTPS();
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CMonitorServer::DBProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 60000);

        if (retval == WAIT_TIMEOUT)
        {

        }
    }

    CloseHandle(dummyevent);

    return true;
}

void procademy::CMonitorServer::LoadInitFile(const WCHAR* fileName)
{
}

void procademy::CMonitorServer::BeginThreads()
{
    mThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
    mThreads[1] = (HANDLE)_beginthreadex(nullptr, 0, DBThread, this, 0, nullptr);
}

void procademy::CMonitorServer::WaitForThreadsFin()
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
        case 'd':
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer Intended Crash\n");
            CRASH();
        case 'q':
            QuitServer();
            return;
        default:
            break;
        }
    }
}

void procademy::CMonitorServer::MakeMonitorStr(WCHAR* s, int size)
{
}

void procademy::CMonitorServer::ClearTPS()
{
}

unsigned int __stdcall procademy::CMonitorServer::MonitorThread(LPVOID arg)
{
    CMonitorServer* server = (CMonitorServer*)arg;

    server->MonitorProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End");

    return 0;
}

unsigned int __stdcall procademy::CMonitorServer::DBThread(LPVOID arg)
{
    CMonitorServer* server = (CMonitorServer*)arg;

    server->DBProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"DB Thread End");

    return 0;
}
