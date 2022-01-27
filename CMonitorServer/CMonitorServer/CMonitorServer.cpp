#include "CMonitorServer.h"
#include "TextParser.h"
#include <conio.h>
#include "CNetPacket.h"
#include "CLanPacket.h"
#include "MonitorProtocol.h"
#include <time.h>
#include "CDBConnector.h"

procademy::CMonitorServer::CMonitorServer()
{
}

procademy::CMonitorServer::~CMonitorServer()
{
    mDB->Disconnect();
    delete mDB;
}

bool procademy::CMonitorServer::BeginServer()
{
    LoadInitFile(L"Server.cnf");
    Begin();
    Init();
    BeginThreads();
    mMonitorToolServer.BeginServer();

    return true;
}

bool procademy::CMonitorServer::RunServer()
{
    HANDLE threads[2] = { mMonitorThread, mDBThread };

    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    mMonitorToolServer.RunServer();

    RunningLoop();

    DWORD ret = WaitForMultipleObjects(2, threads, true, INFINITE);

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
    mDB = new CDBConnector(mLogDBIP, mLogDBUser, mLogDBPassword, mLogDBSchema, mLogDBPort);
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
        DWORD retval = WaitForSingleObjectEx(dummyevent, 60000, true);

        switch (retval)
        {
        case WAIT_TIMEOUT:
            SaveMonitorData();
            break;
        case WAIT_IO_COMPLETION:
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"DB Thread End");
            mDB->Disconnect();
            break;
        default:
            break;
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

    // Server
    tp.GetValue(L"BIND_IP", L"MONITOR_SERVER", buffer);
    SetServerIP(buffer);

    tp.GetValue(L"BIND_PORT", L"MONITOR_SERVER", &num);
    SetServerPort(num);

    tp.GetValue(L"IOCP_WORKER_THREAD", L"MONITOR_SERVER", &num);
    mWorkerThreadNum = (BYTE)num;

    tp.GetValue(L"IOCP_ACTIVE_THREAD", L"MONITOR_SERVER", &num);
    mActiveThreadNum = (BYTE)num;

    tp.GetValue(L"CLIENT_MAX", L"MONITOR_SERVER", &num);
    SetMaxClient(num);

    tp.GetValue(L"NAGLE", L"MONITOR_SERVER", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbNagle = true;
    else
        mbNagle = false;

    tp.GetValue(L"ZERO_COPY", L"MONITOR_SERVER", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbZeroCopy = true;
    else
        mbZeroCopy = false;

    tp.GetValue(L"LOG_LEVEL", L"MONITOR_SERVER", buffer);
    if (wcscmp(buffer, L"DEBUG") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_DEBUG);
    else if (wcscmp(buffer, L"WARNING") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_SYSTEM);
    else if (wcscmp(buffer, L"ERROR") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_ERROR);

    // Service
    tp.GetValue(L"LOG_DB_IP", L"DB_INFO", mLogDBIP);
    tp.GetValue(L"LOG_DB_PORT", L"DB_INFO", &num);
    mLogDBPort = (USHORT)num;

    tp.GetValue(L"LOG_DB_USER", L"DB_INFO", mLogDBUser);
    tp.GetValue(L"LOG_DB_PASS", L"DB_INFO", mLogDBPassword);
    tp.GetValue(L"LOG_DB_SCHEMA", L"DB_INFO", mLogDBSchema);
}

void procademy::CMonitorServer::BeginThreads()
{
    mMonitorThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
    mDBThread = (HANDLE)_beginthreadex(nullptr, 0, DBThread, this, 0, nullptr);
}

void procademy::CMonitorServer::RunningLoop()
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
            QueueUserAPC(APCDBFunc, mDBThread, 0);
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
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"Server Client Num : ", (int)mServerClients.size());
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

    if (value > server->max[data->type])
    {
        server->max[data->type] = value;
    }

    if (value < server->min[data->type])
    {
        server->min[data->type] = value;
    }
}

procademy::CNetPacket* procademy::CMonitorServer::MakeMonitoringPacket(st_ServerClient* server, st_MonitorData* data)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE << server->serverNo << data->type;

    *packet << data->value << data->timeStamp;

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

void procademy::CMonitorServer::SaveMonitorData()
{
    st_DBData dbSets[DATA_SET_SZIE];
    WCHAR timeStr[64];
    WCHAR tableName[64];
    tm t;
    time_t newTime;

    time(&newTime);
    localtime_s(&t, &newTime);

    swprintf_s(timeStr, _countof(timeStr), L"%d-%d-%d %d:%d:%d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    swprintf_s(tableName, _countof(tableName), L"monitorLog_%d_%d", t.tm_year + 1900, t.tm_mon + 1);

    LockServer();
    for (auto iter = mServerClients.begin(); iter != mServerClients.end(); ++iter)
    {
        for (int i = 1; i < DATA_SET_SZIE; ++i)
        {
            CSafeQueue<st_MonitorData*>* dataSet = &iter->second->dataSet[i];

            int sum = 0;
            int timeVal = 0;
            int count = dataSet->GetUseSize();

            for (int j = 0; j < count; ++j)
            {
                st_MonitorData* data = dataSet->Dequeue();

                sum += data->value;
                timeVal = data->timeStamp;

                mMonitorDataPool.Free(data);
            }

            if (0 == count)
            {
                dbSets[i].serverNo = -1;
                continue;
            }
            
            if (count > 2)
            {
                dbSets[i].max = iter->second->max[i];
                dbSets[i].min = iter->second->min[i];
                dbSets[i].avg = (sum - dbSets[i].max - dbSets[i].min) / (count - 2);
                dbSets[i].serverNo = iter->second->serverNo;
                dbSets[i].type = i;
            }
            else
            {
                dbSets[i].max = iter->second->max[i];
                dbSets[i].min = iter->second->min[i];
                dbSets[i].avg = sum / count;
                dbSets[i].serverNo = iter->second->serverNo;
                dbSets[i].type = i;
            }

            iter->second->max[i] = 0;
            iter->second->min[i] = MAXINT32;
        }
    }
    UnlockServer();

    for (int i = 1; i < DATA_SET_SZIE;)
    {
        if (dbSets[i].serverNo >= 0)
        {
            bool retval = mDB->Query_Save(L"INSERT INTO logdb.`%s` (logtime, serverno, type, avr, min, max) VALUES (cast('%s' As datetime), %d, %d, %d, %d, %d)", 
                tableName,
                timeStr,
                dbSets[i].serverNo,
                dbSets[i].type,
                dbSets[i].avg,
                dbSets[i].min,
                dbSets[i].max);

            if (false == retval)
            {
                if (mDB->GetLastError() == 1146)
                {
                    retval = mDB->Query_Save(L"CREATE TABLE logdb.`%s` LIKE logdb.`monitorlog`", tableName);

                    if (false == retval)
                    {
                        CLogger::_Log(dfLOG_LEVEL_ERROR, mDB->GetLastErrorMsg());
                    }

                    continue;
                }
                else
                {
                    CLogger::_Log(dfLOG_LEVEL_ERROR, mDB->GetLastErrorMsg());
                }
            }
        }
        i++;
    }
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
