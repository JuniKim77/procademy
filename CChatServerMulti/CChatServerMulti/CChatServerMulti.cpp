#pragma warning(disable:6387)
#pragma warning(disable:26110)

#define SEND_TO_WORKER

#include "CChatServerMulti.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"
#include "TextParser.h"
#include <vector>
#include <conio.h>
#include "CProfiler.h"
#include <stack>

#define MAX_STR (30000)

WCHAR str[MAX_STR];

procademy::CChatServerMulti::CChatServerMulti()
{
}

procademy::CChatServerMulti::~CChatServerMulti()
{
}

bool procademy::CChatServerMulti::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void procademy::CChatServerMulti::OnClientJoin(SESSION_ID SessionID)
{
    InterlockedIncrement(&mUpdateTPS);
    mRatioMonitor.joinCount++;
#ifdef PROFILE
    CProfiler::Begin(L"JoinProc");
    JoinProc(SessionID);
    CProfiler::End(L"JoinProc");
#else
    JoinProc(SessionID);
#endif
}

void procademy::CChatServerMulti::OnClientLeave(SESSION_ID SessionID)
{
    InterlockedIncrement(&mUpdateTPS);
    InterlockedIncrement(&mRatioMonitor.leaveCount);
#ifdef PROFILE
    CProfiler::Begin(L"LeaveProc");
    LeaveProc(SessionID);
    CProfiler::End(L"LeaveProc");
#else
    LeaveProc(SessionID);
#endif
}

void procademy::CChatServerMulti::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
    InterlockedIncrement(&mUpdateTPS);
    RecvProc(SessionID, packet);
}

void procademy::CChatServerMulti::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CChatServerMulti::BeginServer()
{
    LoadInitFile(L"Server.cnf");
    Begin();
    Init();
    BeginThreads();

	return true;
}

bool procademy::CChatServerMulti::RunServer()
{
    HANDLE handles[3] = { mMonitoringThread, mHeartbeatThread };

    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    RunningLoop();

    DWORD ret = WaitForMultipleObjects(2, handles, true, INFINITE);

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

void procademy::CChatServerMulti::RunningLoop()
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
        case 'r':
            mbPrint = true;
            wprintf(L"Set Print Ratio\n");
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

void procademy::CChatServerMulti::Init()
{
    char    IP[64];
    int     index = 0;

    WideCharToMultiByte(CP_ACP, 0, mTokenDBIP, -1, IP, sizeof(IP), nullptr, nullptr);

    mRedis.connect(IP, mTokenDBPort);

    InitializeSRWLock(&mPlayerMapLock);

    for (int i = 0; i < SECTOR_MAX_Y; ++i)
    {
        for (int j = 0; j < SECTOR_MAX_X; ++j)
        {
            mSector[i][j].lockIndex = index++;
            InitializeSRWLock(&mSector[i][j].sectorLock);            
        }
    }
}

bool procademy::CChatServerMulti::CheckHeartProc()
{
    ULONGLONG               curTime;
    HANDLE                  dummyevent = CreateEvent(nullptr, false, false, nullptr);
    DWORD                   retval;
    SESSION_ID              sessionNo;

    while (!mbExit)
    {
        retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            curTime = GetTickCount64();

            LockPlayerMap(false);
            {
                for (auto iter = mPlayerMap.begin(); iter != mPlayerMap.end(); ++iter)
                {
                    ULONGLONG playerTime = iter->second->lastRecvTime;
                    if (curTime > playerTime)
                    {
                        if (curTime - playerTime > mTimeOut)
                        {
                            Disconnect(iter->second->sessionNo);
                        }
                    }
                }
            }
            UnlockPlayerMap(false);

            return true;
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CChatServerMulti::MonitoringProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
    WCHAR str[2048];

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            mCpuUsage.UpdateProcessorCpuTime();
            RecordPerformentce();
            // 출력
            MakeMonitorStr(str, 2048);

            wprintf(str);

            MakeRatioMonitorStr(str, 2048);

            wprintf(str);

            ClearTPS();
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CChatServerMulti::JoinProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player != nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Player[%llu]", sessionNo);
        CRASH();

        return false;
    }

    player = mPlayerPool.Alloc();
    player->sessionNo = sessionNo;
    player->lastRecvTime = GetTickCount64();

    //msgDebugLog(1000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);

    LockPlayerMap();
    InsertPlayer(sessionNo, player);
    UnlockPlayerMap();

    return true;
}

bool procademy::CChatServerMulti::RecvProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    WORD type;
    bool ret = false;

    *packet >> type;

#ifdef PROFILE
    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        InterlockedIncrement(&mRatioMonitor.loginCount);
        if (mbRedisMode)
        {
            CProfiler::Begin(L"LoginProc_Redis");
            ret = LoginProc_Redis(sessionNo, packet);
            CProfiler::End(L"LoginProc_Redis");
        }
        else
        {
            CProfiler::Begin(L"LoginProc");
            ret = LoginProc(sessionNo, packet);
            CProfiler::End(L"LoginProc");
        }
        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        InterlockedIncrement(&mRatioMonitor.moveSectorCount);
        CProfiler::Begin(L"MoveSectorProc");
        ret = MoveSectorProc(sessionNo, packet);
        CProfiler::End(L"MoveSectorProc");
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:
        InterlockedIncrement(&mRatioMonitor.sendMsgInCount);
        CProfiler::Begin(L"SendMessageProc");
        ret = SendMessageProc(sessionNo, packet);
        CProfiler::End(L"SendMessageProc");
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        CProfiler::Begin(L"HeartUpdateProc");
        ret = HeartUpdateProc(sessionNo);
        CProfiler::End(L"HeartUpdateProc");
        break;
    default:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Undefined Message", sessionNo);
        break;
}
#else
    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        InterlockedIncrement(&mRatioMonitor.loginCount);
        ret = LoginProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        InterlockedIncrement(&mRatioMonitor.moveSectorCount);
        ret = MoveSectorProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:
        InterlockedIncrement(&mRatioMonitor.sendMsgInCount);
        ret = SendMessageProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        ret = HeartUpdateProc(sessionNo);
        break;
    default:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Undefined Message", sessionNo);
        break;
    }
#endif // PROFILE
    

    if (ret == false)
    {
        Disconnect(sessionNo);
    }

    return true;
}

bool procademy::CChatServerMulti::LoginProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	    AccountNo;
    WCHAR	    ID[20];				// null 포함
    WCHAR	    Nickname[20];		// null 포함
    char	    SessionKey[64];		// 인증토큰
    CNetPacket* response;
    char        buffer[12];
    bool        cmpRet = false;

    * packet >> AccountNo;

    packet->GetData(ID, 20);
    packet->GetData(Nickname, 20);
    packet->GetData(SessionKey, 64);

    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - Player[%llu] Not Found", sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != 0 || player->bLogin)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login",
            sessionNo, player->accountNo);

        CRASH();*/

		return false;
	}

	// token verification
    if (mbRedisMode)
    {
        _i64toa_s(AccountNo, buffer, 12, 10);

        mRedis.sync_commit();

        mRedis.get(buffer, [SessionKey, &cmpRet](cpp_redis::reply& reply) {
            cmpRet = strcmp(reply.as_string().c_str(), SessionKey) == 0;
            });

        if (cmpRet == false)
        {
            return false;
        }
    }

	player->accountNo = AccountNo;
	wcscpy_s(player->ID, _countof(player->ID), ID);
	wcscpy_s(player->nickName, _countof(player->nickName), Nickname);
	player->bLogin = true;
	player->lastRecvTime = GetTickCount64();

	//msgDebugLog(2000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);

	response = MakeCSResLogin(1, AccountNo);
	{
#ifdef SEND_TO_WORKER
		SendPacketToWorker(sessionNo, response);
#else
		SendPacket(sessionNo, response);
#endif // SEND_TO_WORKER
	}

	response->SubRef();

    return true;
}

bool procademy::CChatServerMulti::LeaveProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    //msgDebugLog(3000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);

    short curX = player->curSectorX;
    short curY = player->curSectorY;

    if (curX != -1 && curY != -1)
    {
        LockSector(curX, curY);
        {
            Sector_RemovePlayer(curX, curY, player);
        }
        UnlockSector(curX, curY);
    }

    LockPlayerMap();
    DeletePlayer(sessionNo);
    UnlockPlayerMap();

    FreePlayer(player);

    return true;
}

bool procademy::CChatServerMulti::MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	AccountNo;
    WORD	SectorX;
    WORD	SectorY;

    *packet >> AccountNo >> SectorX >> SectorY;

    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

	if (player->accountNo != AccountNo)
	{
		/*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched",
			sessionNo, player->accountNo, AccountNo);

		CRASH();*/

        return false;
    }

	if (SectorX < 0 || SectorY < 0 || SectorX >= SECTOR_MAX_X || SectorY >= SECTOR_MAX_Y)
	{
		/*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [AccountNo %lld] Out of Boundary",
			sessionNo, AccountNo);

		CRASH();*/

		return false;
	}

	short curX = player->curSectorX;
    short curY = player->curSectorY;

    if (curX != -1 && curY != -1)
    {
        LockSectors(curX, curY, SectorX, SectorY);
        {
            Sector_RemovePlayer(curX, curY, player);
            Sector_AddPlayer(SectorX, SectorY, player);
        }
        UnlockSectors(curX, curY, SectorX, SectorY);
    }
    else
    {
        LockSector(SectorX, SectorY);
        {
            Sector_AddPlayer(SectorX, SectorY, player);
        }
        UnlockSector(SectorX, SectorY);
    }

    player->curSectorX = SectorX;
    player->curSectorY = SectorY;
    player->lastRecvTime = GetTickCount64();

    CNetPacket* response = MakeCSResSectorMove(AccountNo, SectorX, SectorY);
    {
#ifdef SEND_TO_WORKER
        SendPacketToWorker(sessionNo, response);
#else
        SendPacket(sessionNo, response);
#endif // SEND_TO_WORKER
    }
    
    response->SubRef();

    return true;
}

bool procademy::CChatServerMulti::SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	            AccountNo;
    WORD                messageLen;
    st_Player*          player;
    st_Sector_Around    sectorAround;

    *packet >> AccountNo >> messageLen;

    player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != AccountNo)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched",
            sessionNo, player->accountNo, AccountNo);*/

            //CRASH();

        return false;
    }

    if (messageLen != packet->GetUseSize())
    {
        return false;
    }

    player->lastRecvTime = GetTickCount64();

    //msgDebugLog(5000, sessionNo, player->accountNo, player->curSectorX, player->curSectorY, player->bLogin);

    short curX = player->curSectorX;
    short curY = player->curSectorY;

    GetSectorAround(curX, curY, &sectorAround);
    
    CNetPacket* response = MakeCSResMessage(player->accountNo, player->ID, player->nickName, messageLen, (WCHAR*)packet->GetFrontPtr());
    
    DWORD count = SendMessageSectorAround(response, &sectorAround);

    InterlockedAdd(&mRatioMonitor.sendMsgOutCount, count);

    response->SubRef();

    return true;
}

bool procademy::CChatServerMulti::HeartUpdateProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"HeartUpdateProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"HeartUpdateProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    player->lastRecvTime = GetTickCount64();

    return true;
}

void procademy::CChatServerMulti::BeginThreads()
{
    mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
    mHeartbeatThread = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
}

void procademy::CChatServerMulti::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
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

    tp.GetValue(L"PACKET_CODE", L"SERVER", &num);
    code = (BYTE)num;
    CNetPacket::SetCode(code);

    tp.GetValue(L"PACKET_KEY", L"SERVER", &num);
    key = (BYTE)num;
    CNetPacket::SetPacketKey(key);

    // SERVICE

#ifdef TLS_MEMORY_POOL_VER
    tp.GetValue(L"POOL_SIZE_CHECK", L"SERVICE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
    {
        CNetPacket::sPacketPool.OnOffCounting();
    }
#endif // TLS_MEMORY_POOL_VER

    tp.GetValue(L"TIMEOUT_DISCONNECT", L"SERVICE", &mTimeOut);

    tp.GetValue(L"TOKEN_DB_IP", L"SERVICE", mTokenDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", L"SERVICE", &num);
    mTokenDBPort = (USHORT)num;

    tp.GetValue(L"REDIS_MODE", L"SERVICE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbRedisMode = true;
    else
        mbRedisMode = false;
}

void procademy::CChatServerMulti::FreePlayer(st_Player* player)
{
    player->accountNo = 0;
    player->curSectorX = -1;
    player->curSectorY = -1;

    player->bLogin = false;

    mPlayerPool.Free(player);
}

procademy::st_Player* procademy::CChatServerMulti::FindPlayer(SESSION_ID sessionNo)
{
    //LockPlayerMap(false);

    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    //UnlockPlayerMap(false);
    

    if (iter == mPlayerMap.end())
    {
        return nullptr;
    }

    return iter->second;
}

void procademy::CChatServerMulti::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    /*LockPlayerMap();
    {
        mPlayerMap[sessionNo] = player;
    }
    UnlockPlayerMap();*/

    mPlayerMap[sessionNo] = player;
}

void procademy::CChatServerMulti::DeletePlayer(SESSION_ID sessionNo)
{
    /*LockPlayerMap();
    {
        mPlayerMap.erase(sessionNo);
    }
    UnlockPlayerMap();*/

    mPlayerMap.erase(sessionNo);
}

void procademy::CChatServerMulti::Sector_AddPlayer(WORD x, WORD y, st_Player* player)
{
    mSector[y][x].list.push_back(player);
}

void procademy::CChatServerMulti::Sector_RemovePlayer(WORD x, WORD y, st_Player* player)
{
    for (auto iter = mSector[y][x].list.begin(); iter != mSector[y][x].list.end();)
    {
        if ((*iter)->sessionNo == player->sessionNo)
        {
            mSector[y][x].list.erase(iter);

            return;
        }
        else
        {
            iter++;
        }
    }

    CLogger::_Log(dfLOG_LEVEL_ERROR, L"DeleteFromSector[X %u][Y %u] - Not Found Player[%lld]",
        x, y, player->accountNo);

    CRASH();
}

void procademy::CChatServerMulti::GetSectorAround(WORD x, WORD y, st_Sector_Around* output)
{
    WORD beginX = x == 0 ? 0 : x - 1;
    WORD beginY = y == 0 ? 0 : y - 1;
    WORD endX = x == 49 ? 49 : x + 1;
    WORD endY = y == 49 ? 49 : y + 1;

    output->count = 0;

    for (WORD i = beginY; i <= endY; ++i)
    {
        for (WORD j = beginX; j <= endX; ++j)
        {
            output->around[output->count].y = i;
            output->around[output->count].x = j;
            output->count++;
        }
    }
}

DWORD procademy::CChatServerMulti::SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input)
{
    std::stack<SESSION_ID>  stk;
    DWORD                   ret = 0;
    DWORD                   size;

    for (int i = 0; i < input->count; ++i)
    {
        int curX = input->around[i].x;
        int curY = input->around[i].y;

        LockSector(curX, curY, false);
        {
            size = (DWORD)mSector[curY][curX].list.size();

            for (std::list<st_Player*>::iterator iter = mSector[curY][curX].list.begin(); iter != mSector[curY][curX].list.end(); ++iter)
            {
                stk.push((*iter)->sessionNo);
            }
        }
        UnlockSector(curX, curY, false);

        while (!stk.empty())
        {
            SESSION_ID sessionNo = stk.top();
            stk.pop();

#ifdef SEND_TO_WORKER
            SendPacketToWorker(sessionNo, packet);
#else
            SendPacket(sessionNo, packet);
#endif // SEND_TO_WORKER
        }
    }

    return ret;
}

void procademy::CChatServerMulti::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Chat Multi Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Player Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
#ifdef TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
#endif // TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Player Pool : ", mPlayerPool.GetCapacity(), mPlayerPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Update TPS : ", mUpdateTPS);
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

void procademy::CChatServerMulti::MakeRatioMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    long total = mRatioMonitor.joinCount + mRatioMonitor.leaveCount + mRatioMonitor.loginCount
        + mRatioMonitor.moveSectorCount + mRatioMonitor.sendMsgInCount;
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"Join Ratio", mRatioMonitor.joinCount, total, mRatioMonitor.joinCount / (float)total);
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"Leave Ratio", mRatioMonitor.leaveCount, total, mRatioMonitor.leaveCount / (float)total);
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"Login Ratio", mRatioMonitor.loginCount, total, mRatioMonitor.loginCount / (float)total);
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"MoveSector Ratio", mRatioMonitor.moveSectorCount, total, mRatioMonitor.moveSectorCount / (float)total);
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"SendReq Ratio", mRatioMonitor.sendMsgInCount, total, mRatioMonitor.sendMsgInCount / (float)total);
    idx += swprintf_s(s + idx, size - idx, L"%22s : (%d / %d) %.2f\n", L"SendMsg Ratio", mRatioMonitor.sendMsgInCount, mRatioMonitor.sendMsgOutCount, mRatioMonitor.sendMsgInCount / (float)mRatioMonitor.sendMsgOutCount);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
}

void procademy::CChatServerMulti::ClearTPS()
{
    mUpdateTPS = 0;

    mRatioMonitor.joinCount = 0;
    mRatioMonitor.loginCount = 0;
    mRatioMonitor.leaveCount = 0;
    mRatioMonitor.moveSectorCount = 0;
    mRatioMonitor.sendMsgInCount = 0;
    mRatioMonitor.sendMsgOutCount = 0;
}

void procademy::CChatServerMulti::RecordPerformentce()
{
    CProfiler::SetRecord(L"Accept_TPS_AVG", (LONGLONG)mMonitor.acceptTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Update_TPS_AVG", (LONGLONG)mUpdateTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Recv_TPS_AVG", (LONGLONG)mMonitor.prevRecvTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Send_TPS_AVG", (LONGLONG)mMonitor.prevSendTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"CPU_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessorTotal(), CProfiler::PROFILE_TYPE::PERCENT);
    CProfiler::SetRecord(L"PROCESS_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessTotal(), CProfiler::PROFILE_TYPE::PERCENT);
}

void procademy::CChatServerMulti::LockSector(WORD x, WORD y, bool exclusive)
{
#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"LockSector_Exclusive");
        AcquireSRWLockExclusive(&mSector[y][x].sectorLock);
        CProfiler::End(L"LockSector_Exclusive");
    }
    else
    {
        CProfiler::Begin(L"LockSector_Shared");
        AcquireSRWLockShared(&mSector[y][x].sectorLock);
        CProfiler::End(L"LockSector_Shared");
    }
#else
    if (exclusive)
    {
        AcquireSRWLockExclusive(&mSector[y][x].sectorLock);
    }
    else
    {
        AcquireSRWLockShared(&mSector[y][x].sectorLock);
    }
#endif // PROFILE
}

void procademy::CChatServerMulti::UnlockSector(WORD x, WORD y, bool exclusive)
{
#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"UnlockSector_Exclusive");
        ReleaseSRWLockExclusive(&mSector[y][x].sectorLock);
        CProfiler::End(L"UnlockSector_Exclusive");
    }
    else
    {
        CProfiler::Begin(L"UnlockSector_Shared");
        ReleaseSRWLockShared(&mSector[y][x].sectorLock);
        CProfiler::End(L"UnlockSector_Shared");
    }
#else
    if (exclusive)
    {
        ReleaseSRWLockExclusive(&mSector[y][x].sectorLock);
    }
    else
    {
        ReleaseSRWLockShared(&mSector[y][x].sectorLock);
    }
#endif // PROFILE
}

void procademy::CChatServerMulti::LockSectors(WORD x1, WORD y1, WORD x2, WORD y2, bool exclusive)
{
    int index1 = mSector[y1][x1].lockIndex;
    int index2 = mSector[y2][x2].lockIndex;

#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"LockSectors_Exclusive");
        if (index1 == index2)
        {
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
        else if (index1 < index2)
        {
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
            AcquireSRWLockExclusive(&mSector[y2][x2].sectorLock);
        }
        else
        {
            AcquireSRWLockExclusive(&mSector[y2][x2].sectorLock);
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
        CProfiler::End(L"LockSectors_Exclusive");
    }
    else
    {
        CProfiler::Begin(L"LockSectors_Shared");
        if (index1 == index2)
        {
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
        }
        else if (index1 < index2)
        {
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
            AcquireSRWLockShared(&mSector[y2][x2].sectorLock);
        }
        else
        {
            AcquireSRWLockShared(&mSector[y2][x2].sectorLock);
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
        }
        CProfiler::End(L"LockSectors_Shared");
    }
#else
    if (exclusive)
    {
        if (index1 == index2)
        {
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
        else if (index1 < index2)
        {
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
            AcquireSRWLockExclusive(&mSector[y2][x2].sectorLock);
}
        else
        {
            AcquireSRWLockExclusive(&mSector[y2][x2].sectorLock);
            AcquireSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
    }
    else
    {
        if (index1 == index2)
        {
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
        }
        else if (index1 < index2)
        {
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
            AcquireSRWLockShared(&mSector[y2][x2].sectorLock);
        }
        else
        {
            AcquireSRWLockShared(&mSector[y2][x2].sectorLock);
            AcquireSRWLockShared(&mSector[y1][x1].sectorLock);
        }
    }
#endif // PROFILE
}

void procademy::CChatServerMulti::UnlockSectors(WORD x1, WORD y1, WORD x2, WORD y2, bool exclusive)
{
    int index1 = mSector[y1][x1].lockIndex;
    int index2 = mSector[y2][x2].lockIndex;

#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"UnlockSectors_Exclusive");
        if (index1 == index2)
        {
            ReleaseSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
        else
        {
            ReleaseSRWLockExclusive(&mSector[y1][x1].sectorLock);
            ReleaseSRWLockExclusive(&mSector[y2][x2].sectorLock);
        }
        CProfiler::End(L"UnlockSectors_Exclusive");
    }
    else
    {
        CProfiler::Begin(L"UnlockSectors_Shared");
        if (index1 == index2)
        {
            ReleaseSRWLockShared(&mSector[y1][x1].sectorLock);
        }
        else
        {
            ReleaseSRWLockShared(&mSector[y1][x1].sectorLock);
            ReleaseSRWLockShared(&mSector[y2][x2].sectorLock);
        }
        CProfiler::End(L"UnlockSectors_Shared");
    }
#else
    if (exclusive)
    {
        if (index1 == index2)
        {
            ReleaseSRWLockExclusive(&mSector[y1][x1].sectorLock);
        }
        else
        {
            ReleaseSRWLockExclusive(&mSector[y1][x1].sectorLock);
            ReleaseSRWLockExclusive(&mSector[y2][x2].sectorLock);
}
    }
    else
    {
        if (index1 == index2)
        {
            ReleaseSRWLockShared(&mSector[y1][x1].sectorLock);
        }
        else
        {
            ReleaseSRWLockShared(&mSector[y1][x1].sectorLock);
            ReleaseSRWLockShared(&mSector[y2][x2].sectorLock);
        }
    }
#endif // PROFILE
}

void procademy::CChatServerMulti::LockPlayerMap(bool exclusive)
{
#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"LockPlayerMap_Exclusive");
        AcquireSRWLockExclusive(&mPlayerMapLock);
        CProfiler::End(L"LockPlayerMap_Exclusive");
}
    else
    {
        CProfiler::Begin(L"LockPlayerMap_Shared");
        AcquireSRWLockShared(&mPlayerMapLock);
        CProfiler::End(L"LockPlayerMap_Shared");
    }
#else
    if (exclusive)
    {
        AcquireSRWLockExclusive(&mPlayerMapLock);
    }
    else
    {
        AcquireSRWLockShared(&mPlayerMapLock);
    }
#endif // PROFILE
}

void procademy::CChatServerMulti::UnlockPlayerMap(bool exclusive)
{
#ifdef PROFILE
    if (exclusive)
    {
        CProfiler::Begin(L"UnlockPlayerMap_Exclusive");
        ReleaseSRWLockExclusive(&mPlayerMapLock);
        CProfiler::End(L"UnlockPlayerMap_Exclusive");
    }
    else
    {
        CProfiler::Begin(L"UnlockPlayerMap_Shared");
        ReleaseSRWLockShared(&mPlayerMapLock);
        CProfiler::End(L"UnlockPlayerMap_Shared");
    }
#else
    if (exclusive)
    {
        ReleaseSRWLockExclusive(&mPlayerMapLock);
    }
    else
    {
        ReleaseSRWLockShared(&mPlayerMapLock);
    }
#endif // PROFILE
    

}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResLogin(BYTE status, SESSION_ID accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << (__int64)accountNo;

    return packet;
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << (__int64)accountNo << sectorX << sectorY;

    return packet;
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE << (__int64)accountNo;

    packet->PutData(ID, 20);
    packet->PutData(nickname, 20);
    *packet << meesageLen;
    packet->PutData(message, meesageLen / 2);

    return packet;
}

unsigned int __stdcall procademy::CChatServerMulti::MonitorFunc(LPVOID arg)
{
    CChatServerMulti* chatServer = (CChatServerMulti*)arg;

    chatServer->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End");

    return 0;
}

unsigned int __stdcall procademy::CChatServerMulti::HeartbeatFunc(LPVOID arg)
{
    CChatServerMulti* chatServer = (CChatServerMulti*)arg;

    while (!chatServer->mbExit)
    {
        chatServer->CheckHeartProc();
    }

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"HeartBeat Thread End");

    return 0;
}
