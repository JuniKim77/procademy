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
}

procademy::CMMOEchoServer::~CMMOEchoServer()
{
    delete[] mPlayers;
    //_aligned_free(mPlayers);
}

void procademy::CMMOEchoServer::AllocSessions(int num)
{
    mPlayers = new CPlayer[num];
    //mPlayers = (CPlayer*)_aligned_malloc(sizeof(CPlayer) * num, 64);

    for (int i = 0; i < num; ++i)
    {
		//new (&mPlayers[i]) CPlayer;
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
    LoadInitFile(L"Server.cnf");
    Begin();
    Init();
    BeginThreads();
    mMonitorClient.BeginClient();
    mMonitorClient.RunClient();

    return true;
}

bool procademy::CMMOEchoServer::RunServer()
{
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    RunningLoop();

    // ���� �ڵ�

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

void procademy::CMMOEchoServer::RunningLoop()
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
    mMonitorThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
}

void procademy::CMMOEchoServer::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    BYTE        code;
    BYTE        key;
    WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.LoadFile(fileName);

    //Server
    tp.GetValue(L"BIND_IP", L"SERVER", buffer);
    SetServerIP(buffer);

    tp.GetValue(L"BIND_PORT", L"SERVER", &num);
    SetServerPort(num);

    tp.GetValue(L"PACKET_CODE", L"SERVER", &num);
    code = (BYTE)num;
    CNetPacket::SetCode(code);

    tp.GetValue(L"PACKET_KEY", L"SERVER", &num);
    key = (BYTE)num;
    CNetPacket::SetPacketKey(key);

    tp.GetValue(L"IOCP_WORKER_THREAD", L"SERVER", &num);
    mWorkerThreadNum = (BYTE)num;

    tp.GetValue(L"IOCP_ACTIVE_THREAD", L"SERVER", &num);
    mActiveThreadNum = (BYTE)num;

    tp.GetValue(L"CLIENT_MAX", L"SERVER", &num);
    SetMaxClient(num);

    tp.GetValue(L"AUTH_MAX_TRANSFER", L"SERVER", &mMaxTransferToAuth);
    tp.GetValue(L"GAME_MAX_TRANSFER", L"SERVER", &mMaxTransferToGame);

    tp.GetValue(L"NAGLE", L"SERVER", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbNagle = true;
    else
        mbNagle = false;

    tp.GetValue(L"ZERO_COPY", L"SERVER", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbZeroCopy = true;
    else
        mbZeroCopy = false;

    tp.GetValue(L"TIMEOUT_DISCONNECT", L"SERVER", &num);
    SetTimeOut(num);

    tp.GetValue(L"LOG_LEVEL", L"SERVER", buffer);
    if (wcscmp(buffer, L"DEBUG") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_DEBUG);
    else if (wcscmp(buffer, L"WARNING") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_SYSTEM);
    else if (wcscmp(buffer, L"ERROR") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_ERROR);

    tp.GetValue(L"MONITOR_SERVER_IP", L"SERVICE", mMonitorIP);

    tp.GetValue(L"MONITOR_SERVER_PORT", L"SERVICE", &num);
    mMonitorPort = (u_short)num;

    tp.GetValue(L"MONITOR_NO", L"SERVICE", &mServerNo);

    tp.GetValue(L"POOL_SIZE_CHECK", L"SERVICE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        CNetPacket::sPacketPool.OnOffCounting();
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

            RecordPerformentce();
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

void procademy::CMMOEchoServer::RecordPerformentce()
{
    CProfiler::SetRecord(L"Accept_TPS_AVG", (LONGLONG)mMonitor.acceptTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Recv_TPS_AVG", (LONGLONG)mMonitor.prevRecvTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Send_TPS_AVG", (LONGLONG)mMonitor.prevSendTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"CPU_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessorTotal(), CProfiler::PROFILE_TYPE::PERCENT);
    CProfiler::SetRecord(L"PROCESS_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessTotal(), CProfiler::PROFILE_TYPE::PERCENT);
    CProfiler::SetRecord(L"Player_AVG", (LONGLONG)mLoginCount, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Auth_Player_AVG", (LONGLONG)mAuthPlayerNum, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Game_Player_AVG", (LONGLONG)mGamePlayerNum, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Send_Loop_AVG", (LONGLONG)mMonitor.prevSendLoopCount, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Auth_Loop_AVG", (LONGLONG)mMonitor.prevAuthLoopCount, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Game_Loop_AVG", (LONGLONG)mMonitor.prevGameLoopCount, CProfiler::PROFILE_TYPE::COUNT);
}
