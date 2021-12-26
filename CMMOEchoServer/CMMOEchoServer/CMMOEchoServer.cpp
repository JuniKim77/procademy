#pragma warning(disable:6387)
#pragma warning(disable:6385)

#include "CMMOEchoServer.h"
#include <conio.h>
#include "CProfiler.h"
#include "TextParser.h"
#include "CLanPacket.h"
#include "MonitorProtocol.h"
#include <time.h>
#include "CNetPacket.h"

procademy::CMMOEchoServer::CMMOEchoServer()
{
    LoadInitFile(L"Server.cnf");
    Init();
    BeginThreads();
}

procademy::CMMOEchoServer::~CMMOEchoServer()
{
    //delete[] mPlayers;
    _aligned_free(mPlayers);
}

void procademy::CMMOEchoServer::AllocSessions(int num)
{
    //mPlayers = new CPlayer[num];
    mPlayers = (CPlayer*)_aligned_malloc(sizeof(CPlayer) * num, 64);

    for (int i = 0; i < num; ++i)
    {
		new (&mPlayers[i]) CPlayer;
        mPlayers[i].SetServer(this);
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

void procademy::CMMOEchoServer::OnAuth_Update()
{
}

void procademy::CMMOEchoServer::OnGame_Update()
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
    CProfiler::InitProfiler(30);

    mMonitorThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
}

void procademy::CMMOEchoServer::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    //WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.LoadFile(fileName);

    tp.GetValue(L"MONITOR_SERVER_IP", mMonitorIP);

    tp.GetValue(L"MONITOR_SERVER_PORT", &num);
    mMonitorPort = (u_short)num;

    tp.GetValue(L"MONITOR_NO", &mServerNo);
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

            if (mMonitorClient.IsLogin())
            {
                SendMonitorDataProc();
            }
            else
            {
                LoginMonitorServer();
            }

            MakeMonitorStr(str, 2048);

            wprintf(str);
        }
    }

    CloseHandle(dummyevent);
}

void procademy::CMMOEchoServer::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[MMO Echo Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"[Auth Max Transfer: %d] [Game Max Transfer: %d]\n", mMaxTransferToAuth, mMaxTransferToGame);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s(%d / %d)\n", L"Session Num : ", joinCount, mMaxClient);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Player Num : ", mLoginCount);
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"Auth Mode Player : ", mAuthPlayerNum);
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"Game Mode Player : ", mGamePlayerNum);
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Packet Pool: ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send Loop Count : ", mMonitor.prevSendLoopCount);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Auth Loop Count : ", mMonitor.prevAuthLoopCount);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Game Loop Count : ", mMonitor.prevGameLoopCount);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"CPU usage [T:%.1f U:%.1f K:%.1f] [Chat T:%.1f U:%.1f K:%.1f]\n",
        mCpuUsage.ProcessorTotal(), mCpuUsage.ProcessorUser(), mCpuUsage.ProcessorKernel(),
        mCpuUsage.ProcessTotal(), mCpuUsage.ProcessUser(), mCpuUsage.ProcessKernel());
    mCpuUsage.GetBigNumberStr(mCpuUsage.ProcessUserMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"ProcessUserMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.ProcessNonPagedMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"ProcessNonPagedMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.AvailableMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"AvailableMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.NonPagedMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NonPagedMemory : ", bigNumber);
    idx += swprintf_s(s + idx, size - idx, L"%25s%d\n", L"ProcessHandleCount : ", mCpuUsage.ProcessHandleCount());
    idx += swprintf_s(s + idx, size - idx, L"%25s%d\n", L"ProcessThreadCount : ", mCpuUsage.ProcessHandleCount());
    mCpuUsage.GetBigNumberStr(mCpuUsage.NetworkRecvBytes(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NetworkRecvBytes : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.NetworkSendBytes(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NetworkSendBytes : ", bigNumber);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
}

procademy::CLanPacket* procademy::CMMOEchoServer::MakeMonitorLogin(int serverNo)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_SS_MONITOR_LOGIN << serverNo;

    packet->SetHeader();

    return packet;
}

procademy::CLanPacket* procademy::CMMOEchoServer::MakeMonitorPacket(BYTE dataType, int dataValue)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    time_t timeval;

    time(&timeval);

    *packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE << dataType << dataValue << (int)timeval;

    packet->SetHeader();

    return packet;
}

void procademy::CMMOEchoServer::LoginMonitorServer()
{
    mMonitorClient.Connect(mMonitorIP, mMonitorPort);

    if (mMonitorClient.IsJoin())
    {
        CLanPacket* packet = MakeMonitorLogin(mServerNo);

        mMonitorClient.SendPacket(packet);

        packet->SubRef();

        mMonitorClient.SetLogin();
    }
}

void procademy::CMMOEchoServer::SendMonitorDataProc()
{
    CLanPacket* runPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_SERVER_RUN, mbBegin);

    mMonitorClient.SendPacket(runPacket);

    runPacket->SubRef();

    CLanPacket* cpuUsagePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_SERVER_CPU, (int)mCpuUsage.ProcessTotal());

    mMonitorClient.SendPacket(cpuUsagePacket);

    cpuUsagePacket->SubRef();

    CLanPacket* cpuUsageMemoryPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_SERVER_MEM, (int)mCpuUsage.ProcessUserMemory() / 1000000); // 1MB

    mMonitorClient.SendPacket(cpuUsageMemoryPacket);

    cpuUsageMemoryPacket->SubRef();

    CLanPacket* sessionNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_SESSION, (int)joinCount);

    mMonitorClient.SendPacket(sessionNumPacket);

    sessionNumPacket->SubRef();

    CLanPacket* authPlayerNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER, mAuthPlayerNum);

    mMonitorClient.SendPacket(authPlayerNumPacket);

    authPlayerNumPacket->SubRef();

    CLanPacket* gamePlayerNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER, mGamePlayerNum);

    mMonitorClient.SendPacket(gamePlayerNumPacket);

    gamePlayerNumPacket->SubRef();

    CLanPacket* acceptTPSPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS, (int)mMonitor.acceptTPS);

    mMonitorClient.SendPacket(acceptTPSPacket);

    acceptTPSPacket->SubRef();

    CLanPacket* recvTPSPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_PACKET_RECV_TPS, (int)mMonitor.prevRecvTPS);

    mMonitorClient.SendPacket(recvTPSPacket);

    recvTPSPacket->SubRef();

    CLanPacket* sendTPSPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_PACKET_SEND_TPS, (int)mMonitor.prevSendTPS);

    mMonitorClient.SendPacket(sendTPSPacket);

    sendTPSPacket->SubRef();

    CLanPacket* dbWriteTPSPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_DB_WRITE_TPS, 0);

    mMonitorClient.SendPacket(dbWriteTPSPacket);

    dbWriteTPSPacket->SubRef();

    CLanPacket* dbMsgQPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_DB_WRITE_MSG, 0);

    mMonitorClient.SendPacket(dbMsgQPacket);

    dbMsgQPacket->SubRef();

    CLanPacket* authFrameQPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS, (int)mMonitor.prevAuthLoopCount);

    mMonitorClient.SendPacket(authFrameQPacket);

    authFrameQPacket->SubRef();

    CLanPacket* gameFrameQPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS, (int)mMonitor.prevGameLoopCount);

    mMonitorClient.SendPacket(gameFrameQPacket);

    gameFrameQPacket->SubRef();

    CLanPacket* gamePacketPoolPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_GAME_PACKET_POOL, (int)CNetPacket::sPacketPool.GetSize());

    mMonitorClient.SendPacket(gamePacketPoolPacket);

    gamePacketPoolPacket->SubRef();

    CLanPacket* serverCpuUsagePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_MONITOR_CPU_TOTAL, (int)mCpuUsage.ProcessorTotal());

    mMonitorClient.SendPacket(serverCpuUsagePacket);

    serverCpuUsagePacket->SubRef();

    CLanPacket* serverNopagedMemoryPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_MONITOR_NONPAGED_MEMORY, (int)mCpuUsage.NonPagedMemory() / 1000000);

    mMonitorClient.SendPacket(serverNopagedMemoryPacket);

    serverNopagedMemoryPacket->SubRef();

    CLanPacket* serverNetworkRecvPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_RECV, (int)mCpuUsage.NetworkRecvBytes() / 1000);

    mMonitorClient.SendPacket(serverNetworkRecvPacket);

    serverNetworkRecvPacket->SubRef();

    CLanPacket* serverNetworkSendPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_MONITOR_NETWORK_SEND, (int)mCpuUsage.NetworkSendBytes() / 1000);

    mMonitorClient.SendPacket(serverNetworkSendPacket);

    serverNetworkSendPacket->SubRef();

    CLanPacket* serverAvailableMemoryPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_MONITOR_AVAILABLE_MEMORY, (int)mCpuUsage.AvailableMemory());

    mMonitorClient.SendPacket(serverAvailableMemoryPacket);

    serverAvailableMemoryPacket->SubRef();
}
