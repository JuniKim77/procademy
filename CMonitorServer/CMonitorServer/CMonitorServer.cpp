#include "CMonitorServer.h"
#include "TextParser.h"
#include <conio.h>
#include "CNetPacket.h"
#include "CLanPacket.h"
#include "MonitorProtocol.h"

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

    mMonitorToolServer.BeginServer();

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
    JoinProc(SessionID);
}

void procademy::CMonitorServer::OnClientLeave(SESSION_ID SessionID)
{
    LeaveProc(SessionID);
}

void procademy::CMonitorServer::OnRecv(SESSION_ID SessionID, CLanPacket* packet)
{
    RecvProc(SessionID, packet);
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
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.LoadFile(fileName);

    tp.GetValue(L"LAN_DB_IP", mDBIP);
    tp.GetValue(L"LAN_DB_PORT", &num);
    mDBPort = (USHORT)num;
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
                mMonitorToolServer.StartMonitorServer();
                wprintf(L"STOP\n");
            }
            else
            {
                Start();
                mMonitorToolServer.StartMonitorServer();
                wprintf(L"RUN\n");
            }
            break;
        case 'd':
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer Intended Crash\n");
            CRASH();
        case 'q':
            mMonitorToolServer.QuitMonitorServer();
            QuitServer();
            return;
        default:
            break;
        }
    }
}

void procademy::CMonitorServer::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[Monitor Lan Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"%22s%llu\n", L"Server Client Num : ", mServerClients.size());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Monitor Tool Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mMonitorToolServer.mWorkerThreadNum, mMonitorToolServer.mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"%22s%llu\n", L"Monitor Client Num : ", mMonitorToolServer.mMonitorClients.size());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Update TPS : ", mUpdateTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
}

void procademy::CMonitorServer::ClearTPS()
{
    mUpdateTPS = 0;
}

bool procademy::CMonitorServer::JoinProc(SESSION_ID sessionID)
{
    st_ServerClient* server = FindServer(sessionID);

    if (server != nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Server[%llu]", sessionID);

        CRASH();
        return false;
    }

    server = new st_ServerClient;
    server->sessionNo = sessionID;

    LockServer();
    InsertServer(sessionID, server);
    UnlockServer();

    InterlockedIncrement(&mUpdateTPS);

    return true;
}

bool procademy::CMonitorServer::RecvProc(SESSION_ID sessionID, CLanPacket* packet)
{
    WORD type;
    bool ret = false;

    InterlockedIncrement(&mUpdateTPS);

    *packet >> type;

    switch (type)
    {
    case en_PACKET_SS_MONITOR_LOGIN:
        ServerLoginProc(sessionID, packet);
        break;
    case en_PACKET_SS_MONITOR_DATA_UPDATE:
        UpdateDataProc(sessionID, packet);
        break;
    default:
        CRASH();
        break;
    }

    return true;
}

bool procademy::CMonitorServer::LeaveProc(SESSION_ID sessionID)
{
    InterlockedIncrement(&mUpdateTPS);

    LockServer();

    st_ServerClient* server = FindServer(sessionID);

    if (server == nullptr)
    {
        UnlockServer();
        CRASH();

        return false;
    }

    DeleteServer(sessionID);

    delete server;

    UnlockServer();

    return true;
}

bool procademy::CMonitorServer::ServerLoginProc(SESSION_ID sessionID, CLanPacket* packet)
{
    int		ServerNo;

    *packet >> ServerNo;

    LockServer();

    st_ServerClient* server = FindServer(sessionID);

    if (server == nullptr)
    {
        UnlockServer();
        CRASH();

        return false;
    }

    server->serverNo = ServerNo;

    UnlockServer();

    return true;
}

bool procademy::CMonitorServer::UpdateDataProc(SESSION_ID sessionID, CLanPacket* packet)
{
    BYTE	DataType;
    int		DataValue;
    int		TimeStamp;

    *packet >> DataType >> DataValue >> TimeStamp;

    st_MonitorData* dataSet = mMonitorDataPool.Alloc();

    dataSet->type = DataType;
    dataSet->value = DataValue;
    dataSet->timeStamp = TimeStamp;

    LockServer();

    st_ServerClient* server = FindServer(sessionID);

    if (server == nullptr)
    {
        UnlockServer();
        mMonitorDataPool.Free(dataSet);
        CRASH();

        return false;
    }

    CNetPacket* dataPacket = MakeMonitoringPacket(server, dataSet);

    mMonitorToolServer.SendDataToAllClinet(dataPacket);

    dataPacket->SubRef();

    EnqueueDataProc(server, dataSet);

    UnlockServer();

    return true;
}

void procademy::CMonitorServer::EnqueueDataProc(st_ServerClient* server, st_MonitorData* data)
{
    int value = data->value;

    server->dataSet[data->type].Enqueue(data);

    if (value > server->max)
    {
        server->max = value;
    }

    if (value < server->min)
    {
        server->min = value;
    }
}

procademy::CNetPacket* procademy::CMonitorServer::MakeMonitoringPacket(st_ServerClient* server, st_MonitorData* data)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE << server->serverNo << data->type;

    *packet << data->value << data->timeStamp;

    packet->SetHeader();
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CMonitorServer::MakeMonitorLoginRes(BYTE Status)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_MONITOR_TOOL_RES_LOGIN << Status;

    return packet;
}

procademy::st_ServerClient* procademy::CMonitorServer::FindServer(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_ServerClient*>::iterator iter = mServerClients.find(sessionNo);

    if (iter == mServerClients.end())
    {
        return nullptr;
    }

    return iter->second;
}

void procademy::CMonitorServer::InsertServer(SESSION_ID sessionNo, st_ServerClient* server)
{
    mServerClients[sessionNo] = server;
}

void procademy::CMonitorServer::DeleteServer(SESSION_ID sessionNo)
{
    mServerClients.erase(sessionNo);
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
