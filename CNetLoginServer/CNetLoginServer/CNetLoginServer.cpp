#include "CNetLoginServer.h"
#include "LoginServerDTO.h"
#include "TextParser.h"
#include "CNetPacket.h"
#include "CLogger.h"
#include "CommonProtocol.h"

procademy::CNetLoginServer::CNetLoginServer()
{
    LoadInitFile(L"LoginServer.cnf");
    Init();
    BeginThreads();
}

procademy::CNetLoginServer::~CNetLoginServer()
{
}

bool procademy::CNetLoginServer::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CNetLoginServer::OnClientJoin(SESSION_ID SessionID)
{
    JoinUserProc(SessionID);
}

void procademy::CNetLoginServer::OnClientLeave(SESSION_ID SessionID)
{
    LeaveUserProc(SessionID);
}

void procademy::CNetLoginServer::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
    WORD type;

    *packet >> type;

    switch (type)
    {
    case en_PACKET_CS_LOGIN_REQ_LOGIN:
        LoginProc(SessionID, packet);
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
    return false;
}

void procademy::CNetLoginServer::WaitForThreadsFin()
{
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

procademy::st_User* procademy::CNetLoginServer::FindUser(SESSION_ID sessionNo)
{
    std::unordered_map<SESSION_ID, st_User*>::iterator iter;
    st_User* user;

    AcquireSRWLockShared(&mUserMapLock);
    {
        iter = mUserMap.find(sessionNo);
        user = nullptr;

        if (iter != mUserMap.end())
        {
            user = iter->second;
        }
    }
    ReleaseSRWLockShared(&mUserMapLock);

    return user;
}

void procademy::CNetLoginServer::InsertUser(SESSION_ID sessionNo, st_User* user)
{
    AcquireSRWLockExclusive(&mUserMapLock);
    {
        mUserMap[sessionNo] = user;
    }
    ReleaseSRWLockExclusive(&mUserMapLock);
}

void procademy::CNetLoginServer::DeleteUser(SESSION_ID sessionNo)
{
    AcquireSRWLockExclusive(&mUserMapLock);
    {
        mUserMap.erase(sessionNo);
    }
    ReleaseSRWLockExclusive(&mUserMapLock);
}

void procademy::CNetLoginServer::BeginThreads()
{
    mhThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);

    mhThreads[1] = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
}

void procademy::CNetLoginServer::Init()
{
    InitializeSRWLock(&mUserMapLock);
    InitializeSRWLock(&mDBConnectorLock);
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
}

void procademy::CNetLoginServer::FreeUser(st_User* user)
{
    user->accountNo = 0;
    user->lastRecvTime = 0;
    user->sessionNo = 0;

    mUserPool.Free(user);
}

bool procademy::CNetLoginServer::JoinUserProc(SESSION_ID sessionNo)
{
    st_User* user = FindUser(sessionNo);

    if (user != nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent User[%llu]\n", sessionNo);
        CRASH();

        return false;
    }

    user = mUserPool.Alloc();
    user->sessionNo = sessionNo;
    user->lastRecvTime = GetTickCount64();

    InsertUser(sessionNo, user);

    return true;
}

bool procademy::CNetLoginServer::LeaveUserProc(SESSION_ID sessionNo)
{
    st_User* user = FindUser(sessionNo);

    if (user == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [Session %llu] Not Found\n",
            sessionNo);

        CRASH();

        return false;
    }

    if (user->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [SessionID %llu]- [User %llu] Not Match\n", sessionNo, user->sessionNo);

        CRASH();

        return false;
    }

    DeleteUser(user->sessionNo);

    FreeUser(user);

    return true;
}

bool procademy::CNetLoginServer::LoginProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	    AccountNo;
    char	    SessionKey[64];		// 인증토큰
    CNetPacket* response;
    st_User*    user = FindUser(sessionNo);

    if (user == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - User[%llu] Not Found\n", sessionNo);

        CRASH();

        return false;
    }

    if (user->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [SessionID %llu]- [User %llu] Not Match\n", sessionNo, user->sessionNo);

        CRASH();

        return false;
    }

    if (user->accountNo != 0)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login\n",
            sessionNo, user->accountNo);

        CRASH();

        return false;
    }

    *packet >> AccountNo;

    packet->GetData(SessionKey, 64);

    // token verification
    bool retval = ReqAccountDB(AccountNo, user);

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

            for (auto iter = mUserMap.begin(); iter != mUserMap.end(); ++iter)
            {
                if (curTime - iter->second->lastRecvTime > mTimeOut) // 40000ms
                {
                    SESSION_ID sessionNo = iter->second->sessionNo;

                    Disconnect(sessionNo);
                }
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
}

void procademy::CNetLoginServer::ClearTPS()
{
}

bool procademy::CNetLoginServer::ReqAccountDB(INT64 accountNo, st_User* output)
{
    WCHAR	    ID[20];				// null 포함
    WCHAR	    Nickname[20];		// null 포함

    return false;
}

procademy::CNetPacket* procademy::CNetLoginServer::MakeCSResLogin(BYTE status, INT64 accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN << accountNo << status;

    //packet->PutData();

    return packet;
}
