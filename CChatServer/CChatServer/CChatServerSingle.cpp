#include "CChatServerSingle.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"
#include "TextParser.h"
#include <vector>

#define MAX_STR (30000)

struct msgDebug
{
    int			logicId;
    INT64	    SessionNo;
    INT64	    AccountNo;
    int			secX;
    int			secY;
    bool        login;
};

WCHAR str[MAX_STR];
USHORT g_msgIdx;
msgDebug g_msgDebugs[USHRT_MAX + 1];
std::unordered_map<INT64, std::vector<int>> g_msgSort;

void msgDebugLog(
    int			logicId,
    INT64	    SessionNo,
    INT64	    AccountNo,
    int			secX,
    int			secY,
    bool        login
)
{
    USHORT index = g_msgIdx++;

    g_msgDebugs[index].logicId = logicId;
    g_msgDebugs[index].SessionNo = SessionNo;
    g_msgDebugs[index].AccountNo = AccountNo;
    g_msgDebugs[index].secX = secX;
    g_msgDebugs[index].secY = secY;
    g_msgDebugs[index].login = login;

	INT64 cur = SessionNo & 0xffffffffffff;

    if (g_msgSort[cur].size() == 0)
    {
        g_msgSort[cur].reserve(16);
    }

    g_msgSort[cur].push_back(logicId);
}

unsigned int __stdcall procademy::CChatServerSingle::UpdateFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    while (!chatServer->mbExit)
    {
        chatServer->GQCSProc();
    }

    wprintf(L"Update Thread End\n");

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::MonitorFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->MonitoringProc();

    wprintf(L"Monitoring Thread End\n");

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::HeartbeatFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    while (!chatServer->mbExit)
    {
        chatServer->CheckHeart();
    }

    wprintf(L"HeartBeat Thread End\n");

    return 0;
}

void procademy::CChatServerSingle::EnqueueMessage(st_MSG* msg)
{
    //mMsgQ.Enqueue(msg);
    PostQueuedCompletionStatus(mIOCP, 1, (ULONG_PTR)msg, 0);
}

void procademy::CChatServerSingle::GQCSProc()
{
    while (1)
    {
        DWORD           transferredSize = 0;
        st_MSG*         msg = nullptr;
        WSAOVERLAPPED*  pOverlapped = nullptr;

        BOOL gqcsRet = GetQueuedCompletionStatus(mIOCP, &transferredSize, (PULONG_PTR)&msg, &pOverlapped, INFINITE);

        // ECHO Server End
        if (transferredSize == 0)
        {
            return;
        }

        mUpdateTPS++;

        //mMsgQ.Dequeue(&msg);

        switch (msg->type)
        {
        case MSG_TYPE_RECV:
            CompleteMessage(msg->sessionNo, msg->packet);
            break;
        case MSG_TYPE_JOIN:
            JoinProc(msg->sessionNo);
            break;
        case MSG_TYPE_LEAVE:
            LeaveProc(msg->sessionNo);
            break;
        case MSG_TYPE_TIMEOUT:
            CheckTimeOutProc();
            break;
        default:
            CLogger::_Log(dfLOG_LEVEL_ERROR, L"GQCSProc - Undefined Message\n");
            break;
        }

        mMsgPool.Free(msg);
    }
}

bool procademy::CChatServerSingle::CheckHeart()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);

    while (mbBegin)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            st_MSG* msg;

            msg = mMsgPool.Alloc();

            msg->type = MSG_TYPE_TIMEOUT;
            msg->sessionNo = 0;
            msg->packet = nullptr;

            EnqueueMessage(msg);
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CChatServerSingle::MonitoringProc()
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

            if (mbPrint)
            {
                PrintRecvSendRatio();
            }

            ClearTPS();
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CChatServerSingle::CompleteMessage(SESSION_ID sessionNo, CNetPacket* packet)
{
    WORD type;

    *packet >> type;

    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        LoginProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        MoveSectorProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:
        SendMessageProc(sessionNo, packet);
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        HeartUpdateProc(sessionNo);
        break;
    default:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Undefined Message\n", sessionNo);
        break;
    }

    packet->SubRef();

    return true;
}

bool procademy::CChatServerSingle::JoinProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player != nullptr)
    {
        //CLogger::_Log(dfLOG_LEVEL_ERROR, L"Concurrent Player[%llu]\n", sessionNo);
        CRASH();

        return false;
    }

    player = mPlayerPool.Alloc();
    player->sessionNo = sessionNo;
    player->lastRecvTime = GetTickCount64();

    msgDebugLog(1000, sessionNo, -1, player->curSectorX, player->curSectorY, player->bLogin);

    InsertPlayer(sessionNo, player);

    return true;
}

bool procademy::CChatServerSingle::LoginProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	INT64	    AccountNo;
	WCHAR	    ID[20];				// null 포함
	WCHAR	    Nickname[20];		// null 포함
	char	    SessionKey[64];		// 인증토큰
    CNetPacket* response;
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - Player[%llu] Not Found\n", sessionNo);

        /*INT64 cur = sessionNo & 0xffffffffffff;

        for (USHORT i = 0; i < g_msgIdx; ++i)
        {
            g_msgSort[g_msgDebugs[i].SessionNo & 0xffffffffffff].push_back(g_msgDebugs[i].logicId);
        }

        std::vector<int>& curlist = g_msgSort[cur];*/
        std::vector<int>& cur = g_msgSort[sessionNo & 0xffffffffffff];
        std::vector<int> notLeaved;
        notLeaved.reserve(5000);

        for (auto iter = g_msgSort.begin(); iter != g_msgSort.end(); ++iter)
        {
            if (iter->second[iter->second.size() - 1] != 3000)
            {
                notLeaved.push_back(iter->first);
            }
        }

        for (int i = 0; i < notLeaved.size(); ++i)
        {
            st_Player* player = mPlayerMap[notLeaved[i]];
            int test = 0;
        }

        CRASH();

        return false;

        /*response = MakeCSResLogin(0, 0);
        SendPacket(sessionNo, response);
        response->SubRef();*/
    }

    if (player->sessionNo != sessionNo)
    {
        CRASH();
    }

    if (player->accountNo != 0 || player->bLogin)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LoginProc - [Session %llu] [pAccountNo %lld] Double Login\n",
            sessionNo, player->accountNo);

        /*response = MakeCSResLogin(0, 0);
        SendPacket(sessionNo, response);
        response->SubRef();*/

        CRASH();

        return false;
    }

    *packet >> AccountNo;

    packet->GetData(ID, 20);
    packet->GetData(Nickname, 20);
    packet->GetData(SessionKey, 64);

    // token verification

    player->accountNo = AccountNo;
    player->sessionNo = sessionNo;
    wcscpy_s(player->ID, NAME_MAX, ID);
    wcscpy_s(player->nickName, NAME_MAX, Nickname);
    //memcpy_s(player->ID, sizeof(player->ID), ID, sizeof(ID));
    //memcpy_s(player->nickName, sizeof(player->nickName), Nickname, sizeof(Nickname));
    player->bLogin = true;
    player->lastRecvTime = GetTickCount64();
    mLoginCount++;

    msgDebugLog(2000, sessionNo, player->accountNo, player->curSectorX, player->curSectorY, player->bLogin);

    response = MakeCSResLogin(1, player->accountNo);
    // 로그?
    SendPacket(player->sessionNo, response);
    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::LeaveProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"LeaveProc - [Session %llu] Not Found\n",
            sessionNo);

        std::vector<int>& cur = g_msgSort[sessionNo & 0xffffffffffff];
        std::vector<int> notLeaved;
        notLeaved.reserve(5000);

        for (auto iter = g_msgSort.begin(); iter != g_msgSort.end(); ++iter)
        {
            if (iter->second[iter->second.size() - 1] != 3000)
            {
                notLeaved.push_back(iter->first);
            }
        }

        for (int i = 0; i < notLeaved.size(); ++i)
        {
            st_Player* player = mPlayerMap[notLeaved[i]];
            int test = 0;
        }

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CRASH();
    }

    msgDebugLog(3000, sessionNo, player->accountNo, player->curSectorX, player->curSectorY, player->bLogin);

    if (player->curSectorX != -1 && player->curSectorY != -1)
    {
        Sector_RemovePlayer(player->curSectorX, player->curSectorY, player);
    }

    DeletePlayer(sessionNo);

    player->accountNo = 0;
    player->lastRecvTime = 0;
    player->curSectorX = -1;
    player->curSectorY = -1;
    if (player->bLogin)
    {
        mLoginCount--;
    }
    player->bLogin = false;
    
    mPlayerPool.Free(player);

    return true;
}

bool procademy::CChatServerSingle::MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [Session %llu] Not Found\n",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CRASH();
    }

    *packet >> AccountNo >> SectorX >> SectorY;

    if (player->accountNo != AccountNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched\n",
            sessionNo, player->accountNo, AccountNo);

        CRASH();

        return false;
    }

    if (SectorX < 0 || SectorY < 0 || SectorX >= SECTOR_MAX_X || SectorY >= SECTOR_MAX_Y)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [AccountNo %lld] Out of Boundary\n",
            sessionNo, AccountNo);

        CRASH();

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

    msgDebugLog(4000, sessionNo, player->accountNo, player->curSectorX, player->curSectorY, player->bLogin);

    CNetPacket* response = MakeCSResSectorMove(player->accountNo, player->curSectorX, player->curSectorY);

    SendPacket(player->sessionNo, response);
    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	            AccountNo;
    WORD                messageLen;
    st_Player*          player = FindPlayer(sessionNo);
    st_Sector_Around    sectorAround;

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] Not Found\n",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CRASH();
    }

    *packet >> AccountNo >> messageLen;

    if (player->accountNo != AccountNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched\n",
            sessionNo, player->accountNo, AccountNo);

        CRASH();

        return false;
    }

    player->lastRecvTime = GetTickCount64();

    msgDebugLog(5000, sessionNo, player->accountNo, player->curSectorX, player->curSectorY, player->bLogin);

    GetSectorAround(player->curSectorX, player->curSectorY, &sectorAround);

    mSector[player->curSectorY][player->curSectorX].recvCount++;

    CNetPacket* response = MakeCSResMessage(player->accountNo, player->ID, player->nickName, messageLen, (WCHAR*)packet->GetFrontPtr());

    mSector[player->curSectorY][player->curSectorX].sendCount += SendMessageSectorAround(response, &sectorAround);

    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::HeartUpdateProc(SESSION_ID sessionNo)
{
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"HeartUpdateProc - [Session %llu] Not Found\n",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CRASH();
    }

    player->lastRecvTime = GetTickCount64();

    return true;
}

bool procademy::CChatServerSingle::CheckTimeOutProc()
{
    ULONGLONG curTime = GetTickCount64();

    for (auto iter = mPlayerMap.begin(); iter != mPlayerMap.end();)
    {
        SESSION_ID sessionNo = iter->second->sessionNo;

        if (curTime - iter->second->lastRecvTime > mTimeOut) // 40000ms
        {
            if (iter->second->curSectorX != -1 && iter->second->curSectorY != -1)
            {
                Sector_RemovePlayer(iter->second->curSectorX, iter->second->curSectorY, iter->second);
            }

            if (iter->second->bLogin)
            {
                mLoginCount--;
            }

            iter = mPlayerMap.erase(iter);

            Disconnect(sessionNo);
        }
        else
        {
            ++iter;
        }
    }

    return true;
}

void procademy::CChatServerSingle::BeginThreads()
{
    mUpdateThread = (HANDLE)_beginthreadex(nullptr, 0, UpdateFunc, this, 0, nullptr);
    mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
    mHeartbeatThread = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
}

procademy::st_Player* procademy::CChatServerSingle::FindPlayer(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo & 0xffffffffffff);

    if (iter == mPlayerMap.end())
        return nullptr;
    else
        return iter->second;
}

void procademy::CChatServerSingle::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    mPlayerMap[sessionNo & 0xffffffffffff] = player;
}

void procademy::CChatServerSingle::DeletePlayer(SESSION_ID sessionNo)
{
    mPlayerMap.erase(sessionNo & 0xffffffffffff);
}

void procademy::CChatServerSingle::Sector_AddPlayer(WORD x, WORD y, st_Player* player)
{
    mSector[y][x].list.push_back(player);
}

void procademy::CChatServerSingle::Sector_RemovePlayer(WORD x, WORD y, st_Player* player)
{
    for (auto iter = mSector[y][x].list.begin(); iter != mSector[y][x].list.end(); ++iter)
    {
        if ((*iter)->accountNo == player->accountNo)
        {
            mSector[y][x].list.erase(iter);

            return;
        }
    }

    CLogger::_Log(dfLOG_LEVEL_ERROR, L"DeleteFromSector[X %u][Y %u] - Not Found Player[%lld]",
        x, y, player->accountNo);

    CRASH();
}

void procademy::CChatServerSingle::GetSectorAround(WORD x, WORD y, st_Sector_Around* output)
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

DWORD procademy::CChatServerSingle::SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input)
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
            SendPacket((*iter)->sessionNo, packet);
        }
    }

    return ret;
}

void procademy::CChatServerSingle::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    int len;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Player Num : ", mLoginCount);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Update Msg Pool : ", mMsgPool.GetCapacity(), mMsgPool.GetSize());
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

void procademy::CChatServerSingle::PrintRecvSendRatio()
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

void procademy::CChatServerSingle::ClearTPS()
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

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResLogin(BYTE status, SESSION_ID accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << (__int64)accountNo;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << (__int64)accountNo << sectorX << sectorY;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message)
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

procademy::CChatServerSingle::CChatServerSingle()
{
    LoadInitFile(L"ChatServer.cnf");
    BeginThreads();
}

procademy::CChatServerSingle::~CChatServerSingle()
{
}

bool procademy::CChatServerSingle::OnConnectionRequest(u_long IP, u_short Port)
{
    return true;
}

void procademy::CChatServerSingle::OnClientJoin(SESSION_ID SessionID)
{
    st_MSG* msg;

    msg = mMsgPool.Alloc();

    msg->type = MSG_TYPE_JOIN;
    msg->sessionNo = SessionID;
    msg->packet = nullptr;

    EnqueueMessage(msg);
}

void procademy::CChatServerSingle::OnClientLeave(SESSION_ID SessionID)
{
    st_MSG* msg;

    msg = mMsgPool.Alloc();

    msg->type = MSG_TYPE_LEAVE;
    msg->sessionNo = SessionID;
    msg->packet = nullptr;

    EnqueueMessage(msg);
}

void procademy::CChatServerSingle::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
    st_MSG* msg;

    msg = mMsgPool.Alloc();

    msg->type = MSG_TYPE_RECV;
    msg->sessionNo = SessionID;
    msg->packet = packet;
    packet->AddRef();

    EnqueueMessage(msg);
}

void procademy::CChatServerSingle::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CChatServerSingle::BeginServer()
{
    HANDLE handles[3] = { mUpdateThread, mMonitoringThread, mHeartbeatThread };

    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error\n");

        return false;
    }

    WaitForThreadsFin();

    PostQueuedCompletionStatus(mIOCP, 0, 0, 0);

    DWORD ret = WaitForMultipleObjects(3, handles, true, INFINITE);

    switch (ret)
    {
    case WAIT_FAILED:
        wprintf_s(L"ChatServer Thread Handle Error\n");
        break;
    case WAIT_TIMEOUT:
        wprintf_s(L"ChatServer Thread Timeout Error\n");
        break;
    case WAIT_OBJECT_0:
        wprintf_s(L"ChatServer None Error\n");
        break;
    default:
        break;
    }

    return true;
}


void procademy::CChatServerSingle::LoadInitFile(const WCHAR* fileName)
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

    tp.GetValue(L"POOL_SIZE_CHECK", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        CNetPacket::sPacketPool.OnOffCounting();

    tp.GetValue(L"TIMEOUT_DISCONNECT", &mTimeOut);

    tp.GetValue(L"IOCP_ACTIVE_THREAD", &num);

    mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, (DWORD)num);
}

