#include "CMonitorServer.h"
#include "TextParser.h"
#include <conio.h>
#include "CNetPacket.h"
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

void procademy::CMonitorServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
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

    tp.GetValue(L"TOKEN_DB_IP", mDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", &num);
    mDBPort = (USHORT)num;

    tp.GetValue(L"LOGIN_KEY", buffer);
    WideCharToMultiByte(CP_ACP, 0, buffer, -1, mLoginSessionKey, sizeof(mLoginSessionKey), NULL, NULL);
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
    mUpdateTPS = 0;
}

bool procademy::CMonitorServer::JoinProc(SESSION_ID sessionID)
{
    InterlockedIncrement(&mUpdateTPS);

    return true;
}

bool procademy::CMonitorServer::RecvProc(SESSION_ID sessionID, CNetPacket* packet)
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
    case en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
        MonitorLoginProc(sessionID, packet);
        break;
    default:
        break;
    }

    return false;
}

bool procademy::CMonitorServer::LeaveProc(SESSION_ID sessionID)
{
    InterlockedIncrement(&mUpdateTPS);

    return false;
}

bool procademy::CMonitorServer::MonitorLoginProc(SESSION_ID sessionID, CNetPacket* packet)
{
    char	LoginSessionKey[32];

    packet->GetData(LoginSessionKey, 32);

    if (strcmp(LoginSessionKey, mLoginSessionKey) == 0)
    {

    }


    st_MonitorClient* monitor = new st_MonitorClient;
    monitor->sessionNo = sessionID;

    LockServer();
    InsertMonitorTool(sessionID, monitor);
    UnlockServer();

    //CNetPacket* packet = MakeMonitorLoginRes()

    return true;
}

bool procademy::CMonitorServer::ServerLoginProc(SESSION_ID sessionID, CNetPacket* packet)
{
    int		ServerNo;

    *packet >> ServerNo;

    st_ServerClient* server = new st_ServerClient;
    server->serverNo = ServerNo;
    server->sessionNo = sessionID;

    LockServer();
    InsertServer(sessionID, server);
    UnlockServer();

    return true;
}

bool procademy::CMonitorServer::UpdateDataProc(SESSION_ID sessionID, CNetPacket* packet)
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



    UnlockServer();

    return true;
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

procademy::st_MonitorClient* procademy::CMonitorServer::FindMonitorTool(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_MonitorClient*>::iterator iter = mMonitorClients.find(sessionNo);

    if (iter == mMonitorClients.end())
    {
        return nullptr;
    }

    return iter->second;
}

void procademy::CMonitorServer::InsertMonitorTool(SESSION_ID sessionNo, st_MonitorClient* monitor)
{
    mMonitorClients[sessionNo] = monitor;
}

void procademy::CMonitorServer::DeleteMonitorTool(SESSION_ID sessionNo)
{
    mMonitorClients.erase(sessionNo);
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
