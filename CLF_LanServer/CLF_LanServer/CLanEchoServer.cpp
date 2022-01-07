#include "CLanEchoServer.h"
#include "CLogger.h"
#include "CLanPacket.h"
#include "CCrashDump.h"
#include <conio.h>

procademy::CLanEchoServer::CLanEchoServer()
{
    Init();
    BeginThreads();
}

procademy::CLanEchoServer::~CLanEchoServer()
{
}

bool procademy::CLanEchoServer::BeginServer()
{
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error\n");

        return false;
    }

    WaitForThreadsFin();

    DWORD ret = WaitForSingleObject(mMonitoringThread, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        wprintf_s(L"EchoServer Thread Handle Error\n");
        break;
    case WAIT_TIMEOUT:
        wprintf_s(L"EchoServer Thread Timeout Error\n");
        break;
    case WAIT_OBJECT_0:
        wprintf_s(L"EchoServer None Error\n");
        break;
    default:
        break;
    }

    return true;
}

unsigned int __stdcall procademy::CLanEchoServer::MonitorFunc(LPVOID arg)
{
    CLanEchoServer* chatServer = (CLanEchoServer*)arg;

    chatServer->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"MonitorFunc Thread End");

    return 0;
}

bool procademy::CLanEchoServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CLanEchoServer::OnClientJoin(SESSION_ID SessionID)
{
	CLanPacket* packet = CLanPacket::AllocAddRef();

	int64_t value = 0x7fffffffffffffff;

	*packet << value;

	packet->SetHeader();

	SendPacket(SessionID, packet);
	InsertSessionID(SessionID);
	packet->SubRef();
}

void procademy::CLanEchoServer::OnClientLeave(SESSION_ID SessionID)
{
    LockMap();
    DeleteSessionID(SessionID);
    UnlockMap();
}

void procademy::CLanEchoServer::OnRecv(SESSION_ID SessionID, CLanPacket* packet)
{
    SendPacket(SessionID, packet);
}

void procademy::CLanEchoServer::OnError(int errorcode, const WCHAR* log)
{
}

void procademy::CLanEchoServer::Init()
{
    InitializeSRWLock(&mSessionLock);
}

void procademy::CLanEchoServer::WaitForThreadsFin()
{
    while (1)
    {
        char ch = _getch();

        switch (ch)
        {
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

bool procademy::CLanEchoServer::MonitoringProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
    WCHAR str[1024];

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            // Ãâ·Â
            MakeMonitorStr(str, 1024);

            wprintf(str);
        }
    }

    CloseHandle(dummyevent);

    return true;
}

void procademy::CLanEchoServer::InsertSessionID(u_int64 sessionNo)
{
    mSessionJoinMap[sessionNo]++;
    if (mSessionJoinMap[sessionNo] > 1)
    {
        CRASH();
    }
}

void procademy::CLanEchoServer::DeleteSessionID(u_int64 sessionNo)
{
    mSessionJoinMap[sessionNo]--;
    if (mSessionJoinMap[sessionNo] == 0)
    {
        mSessionJoinMap.erase(sessionNo);
    }
    else
    {
        CRASH();
    }
}

void procademy::CLanEchoServer::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    int len;

    idx += swprintf_s(s + idx, 1024 - idx, L"\n========================================\n");
    //idx += swprintf_s(s + idx, 1024 - idx, L"");
    idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%lld\n", L"Session Num : ", mSessionJoinMap.size());
    idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
    idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Update TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
}

void procademy::CLanEchoServer::BeginThreads()
{
    mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
}
