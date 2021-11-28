#include "CChatServerMulti.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"
#include "TextParser.h"
#include <vector>
#include <conio.h>
#include "CProfiler.h"

#define MAX_STR (30000)

WCHAR str[MAX_STR];

procademy::CChatServerMulti::CChatServerMulti()
{
    LoadInitFile(L"ChatServer.cnf");
    Init();	
	BeginThreads();
}

procademy::CChatServerMulti::~CChatServerMulti()
{
    for (int i = 0; i < mSectorLockColNum; ++i)
    {
        delete[] mpSectorLock[i];
    }

    delete[] mpSectorLock;
}

bool procademy::CChatServerMulti::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void procademy::CChatServerMulti::OnClientJoin(SESSION_ID SessionID)
{
	JoinProc(SessionID);
}

void procademy::CChatServerMulti::OnClientLeave(SESSION_ID SessionID)
{
	LeaveProc(SessionID);
}

void procademy::CChatServerMulti::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{

}

void procademy::CChatServerMulti::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CChatServerMulti::BeginServer()
{
	HANDLE handles[3] = { mMonitoringThread, mHeartbeatThread, mRedisThread };

	if (Start() == false)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

		return false;
	}

	WaitForThreadsFin();

    PostQueuedCompletionStatus(mRedisIOCP, 0, 0, 0);

	DWORD ret = WaitForMultipleObjects(3, handles, true, INFINITE);

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

void procademy::CChatServerMulti::WaitForThreadsFin()
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
    char IP[64];

    WideCharToMultiByte(CP_ACP, 0, mTokenDBIP, -1, IP, sizeof(IP), nullptr, nullptr);

    mRedis.connect(IP, mTokenDBPort);

    InitializeSRWLock(&mPlayerMapLock);

    mpSectorLock = new SRWLOCK*[mSectorLockColNum];
    for (int i = 0; i < mSectorLockColNum; ++i)
    {
        mpSectorLock[i] = new SRWLOCK[mSectorLockColNum];
        for (int j = 0; j < mSectorLockColNum; ++j)
        {
            InitializeSRWLock(&mpSectorLock[i][j]);            
        }
    }
}

bool procademy::CChatServerMulti::CheckHeartProc()
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

bool procademy::CChatServerMulti::MonitoringProc()
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

    InsertPlayer(sessionNo, player);

    return true;
}

bool procademy::CChatServerMulti::LoginProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    //INT64	    AccountNo;
    //WCHAR	    ID[20];				// null 포함
    //WCHAR	    Nickname[20];		// null 포함
    //char	    SessionKey[64];		// 인증토큰
    //CNetPacket* response;
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - Player[%llu] Not Found", sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != 0 || player->bLogin)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login",
            sessionNo, player->accountNo);

        CRASH();

        return false;
    }

    // token verification
    packet->AddRef();

    EnqueueRedisQ(sessionNo, packet);

    //    *packet >> AccountNo;
    //
    //    packet->GetData(ID, 20);
    //    packet->GetData(Nickname, 20);
    //    packet->GetData(SessionKey, 64);
    //
    //    player->accountNo = AccountNo;
    //    wcscpy_s(player->ID, NAME_MAX, ID);
    //    wcscpy_s(player->nickName, NAME_MAX, Nickname);
    //    player->bLogin = true;
    //    player->lastRecvTime = GetTickCount64();
    //    mLoginCount++;
    //
    //    //msgDebugLog(2000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);
    //
    //    response = MakeCSResLogin(1, player->accountNo);
    //    {
    //#ifdef SEND_TO_WORKER
    //        SendPacketToWorker(player->sessionNo, response);
    //#else
    //        SendPacket(player->sessionNo, response);
    //#endif // SEND_TO_WORKER
    //    }
    //    response->SubRef();

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

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    //msgDebugLog(3000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);

    if (player->curSectorX != -1 && player->curSectorY != -1)
    {
        Sector_RemovePlayer(player->curSectorX, player->curSectorY, player);
    }

    if (player->bLogin)
    {
        mLoginCount--;
    }

    FreePlayer(player);

    DeletePlayer(sessionNo);

    return true;
}

bool procademy::CChatServerMulti::MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	AccountNo;
    WORD	SectorX;
    WORD	SectorY;
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    *packet >> AccountNo >> SectorX >> SectorY;

    if (player->accountNo != AccountNo)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched",
            sessionNo, player->accountNo, AccountNo);*/

            //CRASH();

        return false;
    }

    if (SectorX < 0 || SectorY < 0 || SectorX >= SECTOR_MAX_X || SectorY >= SECTOR_MAX_Y)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [AccountNo %lld] Out of Boundary",
            sessionNo, AccountNo);*/

            //CRASH();

        return false;
    }

    if (player->curSectorX != -1 && player->curSectorY != -1)
    {
        Sector_RemovePlayer(player->curSectorX, player->curSectorY, player);
    }

    player->curSectorX = SectorX;
    player->curSectorY = SectorY;
    Sector_AddPlayer(SectorX, SectorY, player);
    player->lastRecvTime = GetTickCount64();

    //msgDebugLog(4000, sessionNo, player, player->curSectorX, player->curSectorY, player->bLogin);

    CNetPacket* response = MakeCSResSectorMove(player->accountNo, player->curSectorX, player->curSectorY);
    {
#ifdef SEND_TO_WORKER
        SendPacketToWorker(player->sessionNo, response);
#else
        SendPacket(player->sessionNo, response);
#endif // SEND_TO_WORKER
    }
    response->SubRef();

    return true;
}

bool procademy::CChatServerMulti::SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	return false;
}

bool procademy::CChatServerMulti::HeartUpdateProc(SESSION_ID sessionNo)
{
	return false;
}

bool procademy::CChatServerMulti::CheckTimeOutProc()
{
	return false;
}

bool procademy::CChatServerMulti::RedisProc()
{
    while (1)
    {
        DWORD           transferredSize = 0;
        SESSION_ID      sessionNo;
        CNetPacket*     packet = nullptr;
        CNetPacket*     result = nullptr;

        BOOL gqcsRet = GetQueuedCompletionStatus(mRedisIOCP, &transferredSize, (PULONG_PTR)&sessionNo, (LPOVERLAPPED*)&packet, INFINITE);

#ifdef PROFILE
        CProfiler::Begin(L"TokenProc");
#endif // PROFILE

        // Redis Thread End
        if (transferredSize == 0)
        {
            return true;
        }

        st_Player* player = FindPlayer(sessionNo);

        if (player == nullptr)
        {
            // 로그인 요청하고 바로 끊을 수 있다.
            continue;
        }

#ifdef PROFILE
        CProfiler::End(L"TokenProc");
#endif // PROFILE
    }

    return true;
}

void procademy::CChatServerMulti::BeginThreads()
{
    mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
    mHeartbeatThread = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
    mRedisThread = (HANDLE)_beginthreadex(nullptr, 0, RedisFunc, this, 0, nullptr);
}

void procademy::CChatServerMulti::LoadInitFile(const WCHAR* fileName)
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

    tp.GetValue(L"TOKEN_DB_IP", mTokenDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", &num);
    mTokenDBPort = (USHORT)num;

    tp.GetValue(L"SECTOR_LOCK_NUM", &mSectorLockColNum);

}

void procademy::CChatServerMulti::FreePlayer(st_Player* player)
{
    player->accountNo = 0;
    player->lastRecvTime = 0;
    player->curSectorX = -1;
    player->curSectorY = -1;

    player->bLogin = false;

    mPlayerPool.Free(player);
}

procademy::st_Player* procademy::CChatServerMulti::FindPlayer(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    if (iter == mPlayerMap.end())
    {
        return nullptr;
    }

    return iter->second;
}

void procademy::CChatServerMulti::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    AcquireSRWLockExclusive(&mPlayerMapLock);
    {
        mPlayerMap[sessionNo] = player;
    }
    ReleaseSRWLockExclusive(&mPlayerMapLock);
}

void procademy::CChatServerMulti::DeletePlayer(SESSION_ID sessionNo)
{
    AcquireSRWLockExclusive(&mPlayerMapLock);
    {
        mPlayerMap.erase(sessionNo);
    }
    ReleaseSRWLockExclusive(&mPlayerMapLock);
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
    DWORD ret = 0;

    for (int i = 0; i < input->count; ++i)
    {
        int curX = input->around[i].x;
        int curY = input->around[i].y;

        mSector[curY][curX].updateCount++;
        mSector[curY][curX].playerCount += mSector[curY][curX].list.size();
        ret += mSector[curY][curX].list.size();

        for (std::list<st_Player*>::iterator iter = mSector[curY][curX].list.begin(); iter != mSector[curY][curX].list.end(); ++iter)
        {
#ifdef SEND_TO_WORKER
            SendPacketToWorker((*iter)->sessionNo, packet);
#else
            SendPacket((*iter)->sessionNo, packet);
#endif // SEND_TO_WORKER
        }
    }

    return ret;
}

void procademy::CChatServerMulti::MakeMonitorStr(WCHAR* s, int size)
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

void procademy::CChatServerMulti::PrintRecvSendRatio()
{
    mbPrint = false;

    FILE* fout = nullptr;
    int idx = 0;
    st_Sector_Around sectorAround;

    _wfopen_s(&fout, L"sectorRatio.csv", L"w");

    if (fout == nullptr)
        return;

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            if (mSector[i][j].recvCount != 0)
            {
                GetSectorAround(j, i, &sectorAround);
                double avgPlayers = 0.0;

                for (int i = 0; i < sectorAround.count; ++i)
                {
                    int curX = sectorAround.around[i].x;
                    int curY = sectorAround.around[i].y;

                    avgPlayers += mSector[curY][curX].playerCount / (double)mSector[curY][curX].updateCount;
                }

                idx += swprintf_s(str + idx, MAX_STR - idx, L"%.2lf<%.2lf>,",
                    mSector[i][j].sendCount / (double)mSector[i][j].recvCount,
                    avgPlayers);
            }
            else
            {
                idx += swprintf_s(str + idx, MAX_STR - idx, L"%6d,", 0);
            }
        }

        idx += swprintf_s(str + idx, MAX_STR - idx, L"\n");
    }

    fwprintf_s(fout, str);

    fclose(fout);
}

void procademy::CChatServerMulti::ClearTPS()
{
    mUpdateTPS = 0;

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            mSector[i][j].recvCount = 0;
            mSector[i][j].sendCount = 0;
            mSector[i][j].updateCount = 0;
            mSector[i][j].playerCount = 0;
        }
    }
}

void procademy::CChatServerMulti::EnqueueRedisQ(SESSION_ID sessionNo, CNetPacket* packet)
{
    PostQueuedCompletionStatus(mRedisIOCP, 1, (ULONG_PTR)sessionNo, (LPOVERLAPPED)packet);
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResLogin(BYTE status, SESSION_ID accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << (__int64)accountNo;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << (__int64)accountNo << sectorX << sectorY;

    packet->SetHeader(false);
    packet->Encode();

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

    packet->SetHeader(false);
    packet->Encode();

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

unsigned int __stdcall procademy::CChatServerMulti::RedisFunc(LPVOID arg)
{
    CChatServerMulti* chatServer = (CChatServerMulti*)arg;

    chatServer->RedisProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Redis Thread End");

    return 0;
}
