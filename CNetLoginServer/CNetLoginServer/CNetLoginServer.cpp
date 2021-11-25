#include "CNetLoginServer.h"
#include "LoginServerDTO.h"
#include "TextParser.h"
#include "CNetPacket.h"
#include "CLogger.h"
#include "CommonProtocol.h"
#include "CProfiler.h"
#include <conio.h>

procademy::CNetLoginServer::CNetLoginServer()
{
    LoadInitFile(L"Server.cnf");
    Init();
    BeginThreads();
}

procademy::CNetLoginServer::~CNetLoginServer()
{
    if (mDBConnector != nullptr)
    {
        mDBConnector->Disconnect();
        delete mDBConnector;
    }

    mRedis.disconnect();
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
    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error\n");

        return false;
    }

    WaitForThreadsFin();

    DWORD ret = WaitForMultipleObjects(2, mhThreads, true, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Handle Error\n");
        break;
    case WAIT_TIMEOUT:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Timeout Error\n");
        break;
    case WAIT_OBJECT_0:
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer End\n");
        break;
    default:
        break;
    }

    return true;
}

void procademy::CNetLoginServer::WaitForThreadsFin()
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

unsigned int __stdcall procademy::CNetLoginServer::MonitorFunc(LPVOID arg)
{
    CNetLoginServer* server = (CNetLoginServer*)arg;

    server->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End\n");

    return 0;
}

unsigned int __stdcall procademy::CNetLoginServer::HeartbeatFunc(LPVOID arg)
{
    CNetLoginServer* server = (CNetLoginServer*)arg;

    server->CheckHeartProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"HeartBeat Thread End\n");

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

    mDBConnector = new CDBConnector(mAccountDBIP, mAccountDBUser, mAccountDBPassword, mAccountDBSchema, mAccountDBPort);
    
    WideCharToMultiByte(CP_ACP, 0, mTokenDBIP, -1, IP, sizeof(IP), nullptr, nullptr);
    
    mRedis.connect(IP, mTokenDBPort);
}

void procademy::CNetLoginServer::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];
    BYTE        code;
    BYTE        key;

    tp.LoadFile(fileName);

    tp.GetValue(L"PACKET_CODE", &num);
    code = (BYTE)num;
    CNetPacket::SetCode(code);

    tp.GetValue(L"PACKET_KEY", &num);
    key = (BYTE)num;
    CNetPacket::SetPacketKey(key);

#ifdef TLS_MEMORY_POOL_VER
    tp.GetValue(L"POOL_SIZE_CHECK", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        CNetPacket::sPacketPool.OnOffCounting();
#endif // TLS_MEMORY_POOL_VER

    tp.GetValue(L"TIMEOUT_DISCONNECT", &mTimeOut);

    tp.GetValue(L"GAME_SERVER_IP", mGameServerIP);
    tp.GetValue(L"GAME_SERVER_PORT", &num);
    mGameServerPort = (USHORT)num;

    tp.GetValue(L"CHAT_SERVER_IP", mChatServerIP);
    tp.GetValue(L"CHAT_SERVER_PORT", &num);
    mChatServerPort = (USHORT)num;

    tp.GetValue(L"TOKEN_DB_IP", mTokenDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", &num);
    mTokenDBPort = (USHORT)num;

    tp.GetValue(L"ACCOUNT_DB_IP", mAccountDBIP);
    tp.GetValue(L"ACCOUNT_DB_PORT", &num);
    mAccountDBPort = (USHORT)num;

    tp.GetValue(L"ACCOUNT_DB_USER", mAccountDBUser);
    tp.GetValue(L"ACCOUNT_DB_PASS", mAccountDBPassword);
    tp.GetValue(L"ACCOUNT_DB_SCHEMA", mAccountDBSchema);
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

	if (player != nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Player[SessionNo : %llu]\n", sessionNo);
		CRASH();

		return false;
	}

	player = mPlayerPool.Alloc();
	player->sessionNo = sessionNo;
	player->lastRecvTime = GetTickCount64();

	InsertPlayer(sessionNo, player);

    return true;
}

bool procademy::CNetLoginServer::LeaveProc(SESSION_ID sessionNo)
{
	st_Player* player = FindPlayer(sessionNo);

	if (player == nullptr)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [Session %llu] Not Found\n",
			sessionNo);
		CRASH();

		return false;
	}

	if (player->sessionNo != sessionNo)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [SessionID %llu]- [Player %llu] Not Match\n", sessionNo, player->sessionNo);
		CRASH();

		return false;
	}

    FreePlayer(player);

	DeletePlayer(sessionNo);

    return true;
}

bool procademy::CNetLoginServer::LoginProc(SESSION_ID sessionNo, CNetPacket* packet, WCHAR* msg)
{
    INT64	    AccountNo;
    char	    SessionKey[64];		// 인증토큰
    CNetPacket* response;
    st_Player*    player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Player %llu] Not Found\n", sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [SessionID %llu]- [Player %llu] Not Match\n", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != 0)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login\n",
            sessionNo, player->accountNo);

        CRASH();

        return false;
    }

    *packet >> AccountNo;

    packet->GetData(SessionKey, 64);

    player->accountNo = AccountNo;

    // token verification
    bool retval = TokenVerificationProc(AccountNo, SessionKey, player);

    if (retval == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Not Found in DB\n",
            sessionNo, AccountNo);

        CRASH();

        return false;
    }

    response = MakeCSResLogin(1, player->accountNo, player->ID, player->nickName);
    {
        SendPacket(sessionNo, response);
    }
    response->SubRef();

    return true;
}

bool procademy::CNetLoginServer::CheckHeartProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);

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
                    if (curTime - iter->second->lastRecvTime > mTimeOut) // 40000ms
                    {
                        SESSION_ID sessionNo = iter->second->sessionNo;

                        Disconnect(sessionNo);
                    }
                }
            }
            ReleaseSRWLockShared(&mPlayerMapLock);
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
            mCpuUsage.UpdateCpuTime();
            // 출력
            MakeMonitorStr(str, 2048);

            wprintf(str);

            ClearTPS();
        }
    }

    CloseHandle(dummyevent);

    return true;
}

void procademy::CNetLoginServer::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    int len;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Player Num : ", mLoginCount);
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
    idx += swprintf_s(s + idx, size - idx, L"CPU usage [T:%.1f U:%.1f K:%.1f] [Chat:%.1f U:%.1f K%.1f]\n",
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

void procademy::CNetLoginServer::ClearTPS()
{
}

bool procademy::CNetLoginServer::TokenVerificationProc(INT64 accountNo, char* sessionKey, st_Player* output)
{
    MYSQL_ROW   sql_row = NULL;
    char        szAccountNumber[65];

    AcquireSRWLockExclusive(&mDBConnectorLock);
    {
        mDBConnector->Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %lld;", accountNo);
        sql_row = mDBConnector->FetchRow();
        mDBConnector->FreeResult();
    }
    ReleaseSRWLockExclusive(&mDBConnectorLock);

    if (sql_row == NULL)
    {
        return false;
    }

    output->accountNo = atoi(sql_row[0]);
    MultiByteToWideChar(CP_ACP, 0, sql_row[1], -1, output->ID, en_NAME_MAX);
    MultiByteToWideChar(CP_ACP, 0, sql_row[2], -1, output->nickName, en_NAME_MAX);

    _i64toa_s(accountNo, szAccountNumber, sizeof(szAccountNumber), 10);

    mRedis.set(szAccountNumber, sessionKey);

    return true;
}

procademy::CNetPacket* procademy::CNetLoginServer::MakeCSResLogin(BYTE status, INT64 accountNo, const WCHAR* id, const WCHAR* nickName)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN << accountNo << status;

    packet->PutData(id, 20);
    packet->PutData(nickName, 20);
    packet->PutData(mGameServerIP, 16);
    *packet << mGameServerPort;
    packet->PutData(mChatServerIP, 16);
    *packet << mChatServerPort;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}
