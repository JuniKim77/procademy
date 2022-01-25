#pragma comment(lib, "winmm.lib")
#pragma warning(disable:6387)

#include "CNetLoginServer.h"
#include "LoginServerDTO.h"
#include "TextParser.h"
#include "CNetPacket.h"
#include "CLogger.h"
#include "CommonProtocol.h"
#include "CProfiler.h"
#include <conio.h>
#include "Query.h"
#include <stack>
#include "CLanPacket.h"
#include "MonitorProtocol.h"

procademy::CNetLoginServer::CNetLoginServer()
{
}

procademy::CNetLoginServer::~CNetLoginServer()
{
    if (mDBConnector != nullptr)
    {
        delete mDBConnector;
    }

    mRedis.disconnect();

    procademy::CRedis_TLS::DestroyRedis_TLS();
    procademy::CDBConnector_TLS::DestroyDBConnector_TLS();
}

bool procademy::CNetLoginServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CNetLoginServer::OnClientJoin(SESSION_ID SessionID)
{
    JoinProc(SessionID);
}

void procademy::CNetLoginServer::OnClientLeave(SESSION_ID SessionID)
{
    LeaveProc(SessionID);
}

void procademy::CNetLoginServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
    WORD type;
    WCHAR errMsg[ERR_MSG_MAX];

    *packet >> type;

    switch (type)
    {
    case en_PACKET_CS_LOGIN_REQ_LOGIN:
        LoginProc(SessionID, packet, errMsg);
        break;
    default:
        break;
    }
}

void procademy::CNetLoginServer::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CNetLoginServer::BeginServer()
{
    LoadInitFile(L"Server.cnf");
    Begin();
    Init();
    BeginThreads();
    mMonitorClient.BeginClient();
    mMonitorClient.RunClient();

    return true;
}

bool procademy::CNetLoginServer::RunServer()
{
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    RunningLoop();

    DWORD ret = WaitForMultipleObjects(2, mhThreads, true, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Handle Error");
        break;
    case WAIT_TIMEOUT:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Timeout Error");
        break;
    case WAIT_OBJECT_0:
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer End");
        break;
    default:
        break;
    }

    return true;
}

void procademy::CNetLoginServer::RunningLoop()
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
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer Intended Crash");
            CRASH();
        case 'q':
            QuitServer();
            return;
        default:
            break;
        }
    }
}

unsigned int __stdcall procademy::CNetLoginServer::MonitorFunc(LPVOID arg)
{
    CNetLoginServer* server = (CNetLoginServer*)arg;

    server->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End");

    return 0;
}

unsigned int __stdcall procademy::CNetLoginServer::HeartbeatFunc(LPVOID arg)
{
    CNetLoginServer* server = (CNetLoginServer*)arg;

    server->CheckHeartProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"HeartBeat Thread End");

    return 0;
}

procademy::st_Player* procademy::CNetLoginServer::FindPlayer(SESSION_ID sessionNo)
{
    std::unordered_map<SESSION_ID, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    if (iter == mPlayerMap.end())
    {
        return nullptr;
    }

    return iter->second;
}

void procademy::CNetLoginServer::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    AcquireSRWLockExclusive(&mPlayerMapLock);
    {
        mPlayerMap[sessionNo] = player;
    }
    ReleaseSRWLockExclusive(&mPlayerMapLock);
}

void procademy::CNetLoginServer::DeletePlayer(SESSION_ID sessionNo)
{
    AcquireSRWLockExclusive(&mPlayerMapLock);
    {
        mPlayerMap.erase(sessionNo);
    }
    ReleaseSRWLockExclusive(&mPlayerMapLock);
}

void procademy::CNetLoginServer::BeginThreads()
{
    mhThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);

    mhThreads[1] = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
}

void procademy::CNetLoginServer::Init()
{
    char IP[64];

    InitializeSRWLock(&mPlayerMapLock);
    InitializeSRWLock(&mDBConnectorLock);
    InitializeSRWLock(&mRedisLock);
    InitializeSRWLock(&mTimeInfoLock);

    mDBConnector = new CDBConnector(mAccountDBIP, mAccountDBUser, mAccountDBPassword, mAccountDBSchema, mAccountDBPort);
    
    CDBConnector_TLS::InitDBConnector_TLS(mAccountDBIP, mAccountDBUser, mAccountDBPassword, mAccountDBSchema, mAccountDBPort, mWorkerThreadNum);

    WideCharToMultiByte(CP_ACP, 0, mTokenDBIP, -1, IP, sizeof(IP), nullptr, nullptr);
    
    mRedis.connect(IP, mTokenDBPort);

    CRedis_TLS::InitRedis_TLS(mTokenDBIP, mTokenDBPort, mWorkerThreadNum);
}

void procademy::CNetLoginServer::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    int         num2;
    WCHAR       buffer[MAX_PARSER_LENGTH];
    BYTE        code;
    BYTE        key;

    tp.LoadFile(fileName);

    // Server
    tp.GetValue(L"BIND_IP", L"SERVER", buffer);
    SetServerIP(buffer);

    tp.GetValue(L"BIND_PORT", L"SERVER", &num);
    SetServerPort(num);

    tp.GetValue(L"IOCP_WORKER_THREAD", L"SERVER", &num);
    mWorkerThreadNum = (BYTE)num;

    tp.GetValue(L"IOCP_ACTIVE_THREAD", L"SERVER", &num);
    mActiveThreadNum = (BYTE)num;

    tp.GetValue(L"CLIENT_MAX", L"SERVER", &num);
    SetMaxClient(num);

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

    tp.GetValue(L"PACKET_CODE", L"SERVER", &num);
    code = (BYTE)num;
    CNetPacket::SetCode(code);

    tp.GetValue(L"PACKET_KEY", L"SERVER", &num);
    key = (BYTE)num;
    CNetPacket::SetPacketKey(key);

    tp.GetValue(L"LOG_LEVEL", L"SERVER", buffer);
    if (wcscmp(buffer, L"DEBUG") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_DEBUG);
    else if (wcscmp(buffer, L"WARNING") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_SYSTEM);
    else if (wcscmp(buffer, L"ERROR") == 0)
        CLogger::setLogLevel(dfLOG_LEVEL_ERROR);

    // Service
#ifdef TLS_MEMORY_POOL_VER
    tp.GetValue(L"POOL_SIZE_CHECK", L"SERVICE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        CNetPacket::sPacketPool.OnOffCounting();
#endif // TLS_MEMORY_POOL_VER

    tp.GetValue(L"TIMEOUT_DISCONNECT", L"SERVICE", &mTimeOut);

    tp.GetValue(L"TOKEN_DB_IP", L"SERVICE", mTokenDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", L"SERVICE", &num);
    mTokenDBPort = (USHORT)num;

    tp.GetValue(L"ACCOUNT_DB_IP", L"SERVICE", mAccountDBIP);
    tp.GetValue(L"ACCOUNT_DB_PORT", L"SERVICE", &num);
    mAccountDBPort = (USHORT)num;

    tp.GetValue(L"ACCOUNT_DB_USER", L"SERVICE", mAccountDBUser);
    tp.GetValue(L"ACCOUNT_DB_PASS", L"SERVICE", mAccountDBPassword);
    tp.GetValue(L"ACCOUNT_DB_SCHEMA", L"SERVICE", mAccountDBSchema);

    tp.GetValue(L"MONITOR_SERVER_IP", L"SERVICE", mMonitorIP);

    tp.GetValue(L"MONITOR_SERVER_PORT", L"SERVICE", &num);
    mMonitorPort = (u_short)num;

    tp.GetValue(L"MONITOR_NO", L"SERVICE", &mServerNo);

    tp.GetValue(L"TLS_MODE", L"SERVICE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbTlsMode = true;
    else
        mbTlsMode = false;

    // Dummy_1
    tp.GetValue(L"IP", L"DUMMY_1", mDummyServers[0].IP);
    tp.GetValue(L"GAME_SERVER_IP", L"DUMMY_1", mDummyServers[0].GameServerIP);
    tp.GetValue(L"GAME_SERVER_PORT", L"DUMMY_1", &num);
    mDummyServers[0].GameServerPort = (USHORT)num;
    tp.GetValue(L"CHAT_SERVER_IP", L"DUMMY_1", mDummyServers[0].ChatServerIP);
    tp.GetValue(L"CHAT_SERVER_PORT", L"DUMMY_1", &num);
    mDummyServers[0].ChatServerPort = (USHORT)num;

    // Dummy_2
    tp.GetValue(L"IP", L"DUMMY_2", mDummyServers[1].IP);
    tp.GetValue(L"GAME_SERVER_IP", L"DUMMY_2", mDummyServers[1].GameServerIP);
    tp.GetValue(L"GAME_SERVER_PORT", L"DUMMY_2", &num);
    mDummyServers[1].GameServerPort = (USHORT)num;
    tp.GetValue(L"CHAT_SERVER_IP", L"DUMMY_2", mDummyServers[1].ChatServerIP);
    tp.GetValue(L"CHAT_SERVER_PORT", L"DUMMY_2", &num);
    mDummyServers[1].ChatServerPort = (USHORT)num;

    // Lan_Client
    tp.GetValue(L"IOCP_WORKER_THREAD", L"LAN_CLIENT", &num);
    tp.GetValue(L"IOCP_ACTIVE_THREAD", L"LAN_CLIENT", &num2);

    mMonitorClient.SetThreadNum(num, num2);
}

void procademy::CNetLoginServer::FreePlayer(st_Player* player)
{
    player->accountNo = 0;
    player->lastRecvTime = 0;
    player->sessionNo = 0;

    mPlayerPool.Free(player);
}

bool procademy::CNetLoginServer::JoinProc(SESSION_ID sessionNo)
{
	st_Player* player = FindPlayer(sessionNo);
    WCHAR IP[16] = { 0, };

	if (player != nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Player[SessionNo : %llu]", sessionNo);
		CRASH();

		return false;
	}

	player = mPlayerPool.Alloc();
	player->sessionNo = sessionNo;
    player->lastRecvTime = GetTickCount64();
    ULONG sessionIP = GetSessionIP(sessionNo);

    InetNtop(AF_INET, &sessionIP, IP, 16);

    if (wcscmp(IP, mDummyServers[0].IP) == 0)
    {
        player->dummyIndex = 0;
    }
    else
    {
        player->dummyIndex = 1;
    }

	InsertPlayer(sessionNo, player);

    return true;
}

bool procademy::CNetLoginServer::LeaveProc(SESSION_ID sessionNo)
{
	st_Player* player = FindPlayer(sessionNo);

	if (player == nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [Session %llu] Not Found",
			sessionNo);
		CRASH();

		return false;
	}

	if (player->sessionNo != sessionNo)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);
		CRASH();

		return false;
	}

    FreePlayer(player);

	DeletePlayer(sessionNo);

    return true;
}

bool procademy::CNetLoginServer::LoginProc(SESSION_ID sessionNo, CNetPacket* packet, WCHAR* msg)
{
    INT64	        AccountNo;
    char	        SessionKey[65];		// 인증토큰
    CNetPacket*     response;
    st_Player*      player = FindPlayer(sessionNo);
    LARGE_INTEGER   loginBegin;
    LARGE_INTEGER   dbBegin;
    LARGE_INTEGER   redisBegin;
    LARGE_INTEGER   redisEnd;
    LARGE_INTEGER   loginEnd;

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Player %llu] Not Found", sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != 0)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login",
            sessionNo, player->accountNo);

        CRASH();

        return false;
    }

    InterlockedIncrement(&mLoginWaitCount);

    QueryPerformanceCounter(&loginBegin);
    
    player->lastRecvTime = GetTickCount64();

    *packet >> AccountNo;

    packet->GetData(SessionKey, 64);
    SessionKey[64] = '\0';

    player->accountNo = AccountNo;

    // token verification
    bool retval = TokenVerificationProc(AccountNo, SessionKey, player, dbBegin, redisBegin, redisEnd);

    if (retval == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Not Found in DB",
            sessionNo, AccountNo);

        return false;
    }

    response = MakeCSResLogin(1, player->accountNo, player->ID, player->nickName, player->dummyIndex);
    {
        SendPacket(sessionNo, response);
    }
    response->SubRef();

    QueryPerformanceCounter(&loginEnd);

    UpdateTimeInfo(loginBegin.QuadPart, dbBegin.QuadPart, redisBegin.QuadPart, redisEnd.QuadPart, loginEnd.QuadPart);

    return true;
}

bool procademy::CNetLoginServer::CheckHeartProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
    std::stack<SESSION_ID> releaseSessions;

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            ULONGLONG curTime = GetTickCount64();
            AcquireSRWLockShared(&mPlayerMapLock);
            {
                for (auto iter = mPlayerMap.begin(); iter != mPlayerMap.end(); ++iter)
                {
                    ULONGLONG playerTime = iter->second->lastRecvTime;

                    if (curTime > playerTime)
                    {
                        if (curTime - playerTime > mTimeOut) // 40000ms
                        {
                            releaseSessions.push(iter->second->sessionNo);
                        }
                    }
                }
            }
            ReleaseSRWLockShared(&mPlayerMapLock);

            while (!releaseSessions.empty())
            {
                SESSION_ID sessionNo = releaseSessions.top();
                releaseSessions.pop();

                Disconnect(sessionNo);
            }
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CNetLoginServer::MonitoringProc()
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

            if (mLoginCount > 0)
            {
                MakeTimeInfoStr(str, 2048);

                wprintf(str);
            }

            ClearTPS();
        }
    }

    CloseHandle(dummyevent);

    return true;
}

void procademy::CNetLoginServer::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Login Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
#ifdef TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
#endif // TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Player Pool : ", mPlayerPool.GetCapacity(), mPlayerPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"CPU usage [T:%.1f U:%.1f K:%.1f] [Login T:%.1f U:%.1f K:%.1f]\n",
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

void procademy::CNetLoginServer::MakeTimeInfoStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    double loginAvg;
    double dbAvg;
    double redisAvg;

    AcquireSRWLockExclusive(&mTimeInfoLock);
    if (mLoginCount > 2)
    {
        loginAvg = (mLoginTimeSum - mLoginTimeMax - mLoginTimeMin) / (double)(mLoginCount - 2) / 10.0;
        dbAvg = (mDBTimeSum - mDBTimeMax - mDBTimeMin) / (double)(mLoginCount - 2) / 10.0;
        redisAvg = (mRedisTimeSum - mRedisTimeMax - mRedisTimeMin) / (double)(mLoginCount - 2) / 10.0;
    }
    else
    {
        loginAvg = mLoginTimeSum / (double)mLoginCount / 10.0;
        dbAvg = mDBTimeSum / (double)mLoginCount / 10.0;
        redisAvg = mRedisTimeSum / (double)mLoginCount / 10.0;
    }
    ReleaseSRWLockExclusive(&mTimeInfoLock);
    
    idx += swprintf_s(s + idx, size - idx, L"%15s%d\n", L"Login Total : ", mLoginTotal);
    idx += swprintf_s(s + idx, size - idx, L"%15s%d\n", L"Wait Count : ", mLoginWaitCount);
    idx += swprintf_s(s + idx, size - idx, L"%15s%d\n", L"Login Count : ", mLoginCount);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Login Avg : ", loginAvg);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Login Max : ", mLoginTimeMax / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Login Min : ", mLoginTimeMin / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"   DB Avg : ", dbAvg);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"   DB Max : ", mDBTimeMax / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"   DB Min : ", mDBTimeMin / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Redis Avg : ", redisAvg);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Redis Max : ", mRedisTimeMax / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"%15s%.2fus\n", L"Redis Min : ", mRedisTimeMin / 10.0);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");

    CProfiler::SetRecord(L"LOGIN_WAIT_AVG", (LONGLONG)mLoginWaitCount, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"LOGIN_COMPLETE_AVG", (LONGLONG)mLoginCount, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"LOGIN_TIME_AVG", (LONGLONG)loginAvg, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"DB_TIME_AVG", (LONGLONG)dbAvg, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"REDIS_TIME_AVG", (LONGLONG)redisAvg, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"PROCESS_CPU_AVG", (LONGLONG)mCpuUsage.ProcessTotal(), CProfiler::PROFILE_TYPE::PERCENT);
}

void procademy::CNetLoginServer::ClearTPS()
{
    AcquireSRWLockExclusive(&mTimeInfoLock);
    {
        mLoginCount = 0;
        mLoginTimeSum = 0;
        mLoginTimeMax = 0;
        mLoginTimeMin = -1;
        mDBTimeSum = 0;
        mDBTimeMax = 0;
        mDBTimeMin = -1;
        mRedisTimeSum = 0;
        mRedisTimeMax = 0;
        mRedisTimeMin = -1;
    }
    ReleaseSRWLockExclusive(&mTimeInfoLock);
}

bool procademy::CNetLoginServer::TokenVerificationProc(INT64 accountNo, char* sessionKey, st_Player* output, 
    LARGE_INTEGER& beginDBTime, LARGE_INTEGER& beginRedisTime, LARGE_INTEGER& endRedisTime)
{
    MYSQL_ROW   sql_row = NULL;
    char        szAccountNumber[20];
    char        IDcopy[20];
    char        nicknameCopy[20];
    INT64       num;
    bool        ret = true;

    std::string strKey(sessionKey);

	if (mbTlsMode)
	{
        QueryPerformanceCounter(&beginDBTime);

		CDBConnector_TLS::Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %lld;", accountNo);

		sql_row = CDBConnector_TLS::FetchRow();

		if (sql_row == NULL)
		{
            CLogger::_Log(dfLOG_LEVEL_ERROR, L"Select Fail %lld - %s", accountNo, CDBConnector_TLS::GetLastErrorMsg());

            return false;
		}

        //printf("%s - %s - %s\n", sql_row[0], sql_row[1], sql_row[2]);

		strcpy_s(szAccountNumber, 20, sql_row[0]);
        memcpy_s(IDcopy, 20, sql_row[1], 20);
        memcpy_s(nicknameCopy, 20, sql_row[2], 20);
		CDBConnector_TLS::FreeResult();

        QueryPerformanceCounter(&beginRedisTime);

		if (ret)
		{
            CRedis_TLS::SetRedis(szAccountNumber, 30, strKey);
		}
        else
        {
            CLogger::_Log(dfLOG_LEVEL_ERROR, L"Verification Fail %s - %s", szAccountNumber, sessionKey);
        }

        QueryPerformanceCounter(&endRedisTime);
	}
	else
	{
        QueryPerformanceCounter(&beginDBTime);

		AcquireSRWLockExclusive(&mDBConnectorLock);
		do
		{
			mDBConnector->Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %lld;", accountNo);

			sql_row = mDBConnector->FetchRow();

			if (sql_row == NULL)
			{
				ret = false;
				break;
			}

            // printf("%s - %s - %s\n", sql_row[0], sql_row[1], sql_row[2]);

			strcpy_s(szAccountNumber, 20, sql_row[0]);
            memcpy_s(IDcopy, 20, sql_row[1], 20);
            memcpy_s(nicknameCopy, 20, sql_row[2], 20);
			mDBConnector->FreeResult();
		} while (0);
		ReleaseSRWLockExclusive(&mDBConnectorLock);

        if (ret == false)
            return false;

        QueryPerformanceCounter(&beginRedisTime);

		AcquireSRWLockExclusive(&mRedisLock);
		do
		{
			if (ret)
			{
				mRedis.setex(szAccountNumber, 30, strKey);
				mRedis.sync_commit();
			}
            else
            {
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"Verification Fail %s - %s", szAccountNumber, sessionKey);
            }
		} while (0);
		ReleaseSRWLockExclusive(&mRedisLock);

        QueryPerformanceCounter(&endRedisTime);
	}

    InterlockedDecrement(&mLoginWaitCount);

    if (ret)
    {
        output->accountNo = accountNo;
        MultiByteToWideChar(CP_ACP, 0, IDcopy, -1, output->ID, en_NAME_MAX);
        MultiByteToWideChar(CP_ACP, 0, nicknameCopy, -1, output->nickName, en_NAME_MAX);
    }

    return ret;
}

void procademy::CNetLoginServer::UpdateTimeInfo(ULONGLONG loginBegin, ULONGLONG dbBegin, ULONGLONG redisBegin, ULONGLONG redisEnd, ULONGLONG loginEnd)
{
    ULONGLONG loginTime = loginEnd - loginBegin;
    ULONGLONG dbTime = redisBegin - dbBegin;
    ULONGLONG redisTime = redisEnd - redisBegin;

    AcquireSRWLockExclusive(&mTimeInfoLock);
    {
        mLoginTimeSum += loginTime;
        mDBTimeSum += dbTime;
        mRedisTimeSum += redisTime;

        if (loginTime > mLoginTimeMax)
            mLoginTimeMax = loginTime;

        if (loginTime < mLoginTimeMin)
            mLoginTimeMin = loginTime;

        if (dbTime > mDBTimeMax)
            mDBTimeMax = dbTime;

        if (loginTime < mDBTimeMin)
            mDBTimeMin = loginTime;

        if (redisTime > mRedisTimeMax)
            mRedisTimeMax = redisTime;

        if (loginTime < mRedisTimeMin)
            mRedisTimeMin = loginTime;

        mLoginCount++;
        mLoginTotal++;
    }
    ReleaseSRWLockExclusive(&mTimeInfoLock);
}

void procademy::CNetLoginServer::LoginMonitorServer()
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

void procademy::CNetLoginServer::SendMonitorDataProc()
{
    CLanPacket* runPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_SERVER_RUN, mbBegin);

    mMonitorClient.SendPacket(runPacket);

    runPacket->SubRef();

    CLanPacket* cpuUsagePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_SERVER_CPU, (int)mCpuUsage.ProcessTotal());

    mMonitorClient.SendPacket(cpuUsagePacket);

    cpuUsagePacket->SubRef();

    CLanPacket* cpuUsageMemoryPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_SERVER_MEM, (int)mCpuUsage.ProcessUserMemory() / 1000000); // 1MB

    mMonitorClient.SendPacket(cpuUsageMemoryPacket);

    cpuUsageMemoryPacket->SubRef();

    CLanPacket* sessionNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_SESSION, (int)mPlayerMap.size());

    mMonitorClient.SendPacket(sessionNumPacket);

    sessionNumPacket->SubRef();

    CLanPacket* authNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS, (int)mLoginCount);

    mMonitorClient.SendPacket(authNumPacket);

    authNumPacket->SubRef();

    CLanPacket* loginPacketPoolPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL, (int)CNetPacket::sPacketPool.GetSize());

    mMonitorClient.SendPacket(loginPacketPoolPacket);

    loginPacketPoolPacket->SubRef();
}

procademy::CNetPacket* procademy::CNetLoginServer::MakeCSResLogin(BYTE status, INT64 accountNo, const WCHAR* id, const WCHAR* nickName, BYTE index)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN << accountNo << status;

    packet->PutData(id, 20);
    packet->PutData(nickName, 20);
    packet->PutData(mDummyServers[index].GameServerIP, 16);
    *packet << mDummyServers[index].GameServerPort;
    packet->PutData(mDummyServers[index].ChatServerIP, 16);
    *packet << mDummyServers[index].ChatServerPort;

    return packet;
}

procademy::CLanPacket* procademy::CNetLoginServer::MakeMonitorLogin(int serverNo)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_SS_MONITOR_LOGIN << serverNo;

    return packet;
}

procademy::CLanPacket* procademy::CNetLoginServer::MakeMonitorPacket(BYTE dataType, int dataValue)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    time_t timeval;

    time(&timeval);

    *packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE << dataType << dataValue << (int)timeval;

    return packet;
}
