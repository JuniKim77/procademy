#pragma warning(disable: 6387)

#define SEND_TO_WORKER
#define REDIS_MODE

#include "CChatServerSingle.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"
#include "TextParser.h"
#include <vector>
#include <conio.h>
#include "CProfiler.h"
#include "CLanPacket.h"
#include "MonitorProtocol.h"

#define MAX_STR (30000)

//struct msgDebug
//{
//    int			logicId;
//    INT64	    SessionNo;
//    void*	    address;
//    int			secX;
//    int			secY;
//    bool        login;
//};
//
//struct info
//{
//    int logicId;
//    INT64 sessionNo;
//};
//
//USHORT g_msgIdx;
//msgDebug g_msgDebugs[USHRT_MAX + 1];
////std::unordered_map<INT64, std::vector<info>> g_msgSort;
////std::unordered_map<INT64, std::vector<info>> g_pointerSort;
//
//void msgDebugLog(
//    int			logicId,
//    INT64	    SessionNo,
//    void*       address,
//    int			secX,
//    int			secY,
//    bool        login
//)
//{
//    USHORT index = g_msgIdx++;
//
//    g_msgDebugs[index].logicId = logicId;
//    g_msgDebugs[index].SessionNo = SessionNo;
//    g_msgDebugs[index].address = address;
//    g_msgDebugs[index].secX = secX;
//    g_msgDebugs[index].secY = secY;
//    g_msgDebugs[index].login = login;
//
//	//INT64 cur = SessionNo;
//
// //   if (g_msgSort[cur].size() == 0)
// //   {
// //       g_msgSort[cur].reserve(16);
// //   }
//
// //   g_msgSort[cur].push_back({ logicId, (INT64)address });
//
// //   g_pointerSort[(INT64)address].push_back({ logicId, SessionNo });
//}

procademy::CChatServerSingle::CChatServerSingle()
{
    LoadInitFile(L"Server.cnf");
    Init();
    BeginThreads();
}

procademy::CChatServerSingle::~CChatServerSingle()
{
    CloseHandle(mUpdateEvent);
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
    packet->AddRef();
    msg->packet = packet;

    EnqueueMessage(msg);
}

void procademy::CChatServerSingle::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CChatServerSingle::BeginServer()
{
    HANDLE handles[4] = { mUpdateThread, mMonitoringThread, mHeartbeatThread, mRedisThread };

    if (Start() == false)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error");

        return false;
    }

    WaitForThreadsFin();

    PostQueuedCompletionStatus(mIOCP, 0, 0, 0);
    PostQueuedCompletionStatus(mRedisIOCP, 0, 0, 0);

    DWORD ret = WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, true, INFINITE);

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

void procademy::CChatServerSingle::WaitForThreadsFin()
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

unsigned int __stdcall procademy::CChatServerSingle::UpdateFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    while (!chatServer->mbExit)
    {
        if (chatServer->mGQCSEx)
            chatServer->GQCSProcEx();
        else
            chatServer->EventProc();
    }

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Update Thread End");

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::MonitorFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End");

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::HeartbeatFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->CheckHeartProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"HeartBeat Thread End");

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::RedisFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->RedisProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Redis Thread End");

    return 0;
}

void procademy::CChatServerSingle::EnqueueMessage(st_MSG* msg)
{
#ifdef PROFILE
    CProfiler::Begin(L"MsgEnqueue");
    if (mGQCSEx)
    {
        PostQueuedCompletionStatus(mIOCP, 1, (ULONG_PTR)msg, 0);
    }
    else
    {
        mMsgQ.Enqueue(msg);

        SetEvent(mUpdateEvent);
    }    
    CProfiler::End(L"MsgEnqueue");
#else
    if (mGQCSEx)
    {
        PostQueuedCompletionStatus(mIOCP, 1, (ULONG_PTR)msg, 0);
    }
    else
    {
        mMsgQ.Enqueue(msg);

        SetEvent(mUpdateEvent);
    }
#endif // PROFILE
}

void procademy::CChatServerSingle::EnqueueRedisQ(SESSION_ID sessionNo, CNetPacket* packet)
{
    PostQueuedCompletionStatus(mRedisIOCP, 1, (ULONG_PTR)sessionNo, (LPOVERLAPPED)packet);
}

void procademy::CChatServerSingle::GQCSProcEx()
{
    OVERLAPPED_ENTRY* overlappedArray = new OVERLAPPED_ENTRY[mGQCSCExNum];

    while (1)
    {
        ULONG               dequeueSize = 0;
        st_MSG*             msg = nullptr;

        BOOL gqcsexRet = GetQueuedCompletionStatusEx(mIOCP, overlappedArray, mGQCSCExNum, &dequeueSize, INFINITE, false);
#ifdef PROFILE
        CProfiler::Begin(L"GQCSProcEx_Chat");
        mLoopCount++;

        for (ULONG i = 0; i < dequeueSize; ++i)
        {
            if (overlappedArray[i].dwNumberOfBytesTransferred == 0)
            {
                delete[] overlappedArray;
                return;
            }

            msg = (st_MSG*)overlappedArray[i].lpCompletionKey;

            mUpdateTPS++;

            switch (msg->type)
            {
            case MSG_TYPE_RECV:
                CompleteMessage(msg->sessionNo, msg->packet);
                break;
            case MSG_TYPE_JOIN:
                CProfiler::Begin(L"JoinProc");          
                mRatioMonitor.joinCount++;
                JoinProc(msg->sessionNo);
                CProfiler::End(L"JoinProc");                
                break;
            case MSG_TYPE_LEAVE:
                CProfiler::Begin(L"LeaveProc");
                InterlockedIncrement(&mRatioMonitor.leaveCount);
                LeaveProc(msg->sessionNo);
                CProfiler::End(L"LeaveProc");
                break;
            case MSG_TYPE_TIMEOUT:
                CheckTimeOutProc();
                break;
            case MSG_TYPE_VERIFICATION_SUCCESS:
                CompleteLoginProc(msg->sessionNo, msg->packet, true);
                break;
            case MSG_TYPE_VERIFICATION_FAIL:
                CompleteLoginProc(msg->sessionNo, msg->packet, false);
                break;
            default:
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"GQCSProc - Undefined Message");
                break;
            }

            mMsgPool.Free(msg);
        }
        CProfiler::End(L"GQCSProcEx_Chat");
#else
        mLoopCount++;

        for (ULONG i = 0; i < dequeueSize; ++i)
        {
            if (overlappedArray[i].dwNumberOfBytesTransferred == 0)
            {
                delete[] overlappedArray;
                return;
            }

            msg = (st_MSG*)overlappedArray[i].lpCompletionKey;

            mUpdateTPS++;

            switch (msg->type)
            {
            case MSG_TYPE_RECV:
                CompleteMessage(msg->sessionNo, msg->packet);
                break;
            case MSG_TYPE_JOIN:            
                mRatioMonitor.joinCount++;
                JoinProc(msg->sessionNo);
                break;
            case MSG_TYPE_LEAVE:
                InterlockedIncrement(&mRatioMonitor.leaveCount);
                LeaveProc(msg->sessionNo);
                break;
            case MSG_TYPE_TIMEOUT:
                CheckTimeOutProc();
                break;
            case MSG_TYPE_VERIFICATION_SUCCESS:
                CompleteLoginProc(msg->sessionNo, msg->packet, true);
                break;
            case MSG_TYPE_VERIFICATION_FAIL:
                CompleteLoginProc(msg->sessionNo, msg->packet, false);
                break;
            default:
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"GQCSProcEx - Undefined Message");
                break;
            }

            mMsgPool.Free(msg);
        }
#endif // PROFILE
    }

    delete[] overlappedArray;
}

void procademy::CChatServerSingle::EventProc()
{
    while (!mbExit)
    {
        st_MSG*             msg = nullptr;

        DWORD retval = WaitForSingleObject(mUpdateEvent, INFINITE);

#ifdef PROFILE
        CProfiler::Begin(L"EventProc");
        mLoopCount++;

        while (mMsgQ.IsEmpty() == false)
        {
            mMsgQ.Dequeue(&msg);
            //msg = mMsgLQ.Dequeue();

            mUpdateTPS++;

            switch (msg->type)
            {
            case MSG_TYPE_RECV:
                CompleteMessage(msg->sessionNo, msg->packet);
                break;
            case MSG_TYPE_JOIN:
                CProfiler::Begin(L"JoinProc");            
                JoinProc(msg->sessionNo);
                CProfiler::End(L"JoinProc");
                break;
            case MSG_TYPE_LEAVE:
                CProfiler::Begin(L"LeaveProc");
                LeaveProc(msg->sessionNo);
                CProfiler::End(L"LeaveProc");
                break;
            case MSG_TYPE_TIMEOUT:
                CheckTimeOutProc();
                break;
            case MSG_TYPE_VERIFICATION_SUCCESS:
                CompleteLoginProc(msg->sessionNo, msg->packet, true);
                break;
            case MSG_TYPE_VERIFICATION_FAIL:
                CompleteLoginProc(msg->sessionNo, msg->packet, false);
                break;
            default:
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"GQCSProc - Undefined Message");
                break;
            }

            mMsgPool.Free(msg);
        }
        CProfiler::End(L"EventProc");
#else
        mLoopCount++;

        while (mMsgQ.IsEmpty() == false)
        {
            mMsgQ.Dequeue(&msg);

            mUpdateTPS++;

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
            case MSG_TYPE_VERIFICATION_SUCCESS:
                CompleteLoginProc(msg->sessionNo, msg->packet, true);
                break;
            case MSG_TYPE_VERIFICATION_FAIL:
                CompleteLoginProc(msg->sessionNo, msg->packet, false);
                break;
            default:
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"GQCSProc - Undefined Message");
                break;
            }

            mMsgPool.Free(msg);
        }
#endif // PROFILE
    }
}

bool procademy::CChatServerSingle::CheckHeartProc()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);

    while (!mbExit)
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
            mCpuUsage.UpdateProcessorCpuTime();
            RecordPerformence();

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

            MakeRatioMonitorStr(str, 2048);

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
    bool ret = false;

    *packet >> type;

#ifdef PROFILE
    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        CProfiler::Begin(L"LoginProc");
        ret = LoginProc(sessionNo, packet);
        CProfiler::End(L"LoginProc");  
        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        CProfiler::Begin(L"MoveSectorProc");
        ret = MoveSectorProc(sessionNo, packet);
        CProfiler::End(L"MoveSectorProc");
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:
        CProfiler::Begin(L"SendMessageProc");
        ret = SendMessageProc(sessionNo, packet);
        CProfiler::End(L"SendMessageProc");
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        ret = HeartUpdateProc(sessionNo);
        break;
    default:
        //CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Undefined Message", sessionNo);
        break;
    }
#else
    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        if (mbRedisMode)
            ret = LoginProc_Redis(sessionNo, packet);
        else
            ret = LoginProc(sessionNo, packet); 
        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        ret = MoveSectorProc(sessionNo, packet); 
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:  
        ret = SendMessageProc(sessionNo, packet);  
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        ret = HeartUpdateProc(sessionNo);
        break;
    default:
        //CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Undefined Message", sessionNo);
        break;
    }
#endif // PROFILE

    packet->SubRef();

    if (ret == false)
    {
        Disconnect(sessionNo);
    }

    return true;
}

bool procademy::CChatServerSingle::JoinProc(SESSION_ID sessionNo)
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

    InterlockedIncrement(&mRatioMonitor.loginCount);

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
	* packet >> AccountNo;

    packet->GetData(ID, 20);
    packet->GetData(Nickname, 20);
    packet->GetData(SessionKey, 64);

    player->accountNo = AccountNo;
    wcscpy_s(player->ID, _countof(player->ID), ID);
    wcscpy_s(player->nickName, _countof(player->nickName), Nickname);
    player->bLogin = true;
    player->lastRecvTime = GetTickCount64();
    mLoginCount++;

    response = MakeCSResLogin(1, player->accountNo);
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

bool procademy::CChatServerSingle::LoginProc_Redis(SESSION_ID sessionNo, CNetPacket* packet)
{
    st_Player* player = FindPlayer(sessionNo);

    InterlockedIncrement(&mRatioMonitor.loginCount);

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

    return true;
}

bool procademy::CChatServerSingle::LeaveProc(SESSION_ID sessionNo)
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

bool procademy::CChatServerSingle::MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
    st_Player* player = FindPlayer(sessionNo);

    InterlockedIncrement(&mRatioMonitor.moveSectorCount);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"MoveSectorProc - [SessionID %llu]- [Player %llu] Not Match", 
            sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    *packet >> AccountNo >> SectorX >> SectorY;

    if (player->accountNo != AccountNo)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched",
            sessionNo, player->accountNo, AccountNo);*/

        return false;
    }

    if (SectorX < 0 || SectorY < 0 || SectorX >= SECTOR_MAX_X || SectorY >= SECTOR_MAX_Y)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [AccountNo %lld] Out of Boundary",
            sessionNo, AccountNo);*/

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

bool procademy::CChatServerSingle::SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet)
{
    INT64	            AccountNo;
    WORD                messageLen;
    st_Player*          player = FindPlayer(sessionNo);
    st_Sector_Around    sectorAround;

    InterlockedIncrement(&mRatioMonitor.sendMsgInCount);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] Not Found",
            sessionNo);

        CRASH();

        return false;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    *packet >> AccountNo >> messageLen;

    if (player->accountNo != AccountNo)
    {
        /*CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched",
            sessionNo, player->accountNo, AccountNo);*/

        return false;
    }

    if (messageLen != packet->GetUseSize())
    {
        return false;
    }

    player->lastRecvTime = GetTickCount64();

    GetSectorAround(player->curSectorX, player->curSectorY, &sectorAround);

    mSector[player->curSectorY][player->curSectorX].recvCount++;

    CNetPacket* response = MakeCSResMessage(player->accountNo, player->ID, player->nickName, messageLen, (WCHAR*)packet->GetFrontPtr());
    {
        DWORD count = SendMessageSectorAround(response, &sectorAround);
        mSector[player->curSectorY][player->curSectorX].sendCount += count;
        InterlockedAdd(&mRatioMonitor.sendMsgOutCount, count);
    }
    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::HeartUpdateProc(SESSION_ID sessionNo)
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

bool procademy::CChatServerSingle::CheckTimeOutProc()
{
    ULONGLONG curTime = GetTickCount64();

    for (auto iter = mPlayerMap.begin(); iter != mPlayerMap.end(); ++iter)
    {
        ULONGLONG playerTime = iter->second->lastRecvTime;

        if (curTime > playerTime)
        {
            if (curTime - playerTime > mTimeOut) // 40000ms
            {
                SESSION_ID sessionNo = iter->second->sessionNo;

                Disconnect(sessionNo);
            }
        }
    }

    return true;
}

bool procademy::CChatServerSingle::CompleteLoginProc(SESSION_ID sessionNo, CNetPacket* packet, bool success)
{
    INT64	    AccountNo;
    WCHAR	    ID[en_NAME_MAX];			// null 포함
    WCHAR	    Nickname[en_NAME_MAX];		// null 포함
    CNetPacket* response;
    st_Player* player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        packet->SubRef();
        return true;
    }

    if (player->sessionNo != sessionNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CompleteLoginProc - [SessionID %llu]- [Player %llu] Not Match", sessionNo, player->sessionNo);

        CRASH();

        return false;
    }

    if (player->accountNo != 0 || player->bLogin)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CompleteLoginProc - [Session %llu] [pAccountNo %lld] Concurrent Login",
            sessionNo, player->accountNo);

        CRASH();

        return false;
    }

	*packet >> AccountNo;

    if (success)
    {
        packet->GetData(ID, en_NAME_MAX);
        packet->GetData(Nickname, en_NAME_MAX);

        player->accountNo = AccountNo;
        wcscpy_s(player->ID, en_NAME_MAX, ID);
        wcscpy_s(player->nickName, en_NAME_MAX, Nickname);
        player->bLogin = true;
        player->lastRecvTime = GetTickCount64();
        mLoginCount++;

#ifdef SEND_TO_WORKER
        response = MakeCSResLogin(1, player->accountNo);
        {
            SendPacketToWorker(player->sessionNo, response);
        }
        response->SubRef();
    }
    else
    {
        response = MakeCSResLogin(0, AccountNo);
        {
            SendPacketToWorker(sessionNo, response);
        }
        response->SubRef();
#else
        response = MakeCSResLogin(1, player->accountNo);
        {
            SendPacket(player->sessionNo, response);
        }
        response->SubRef();
    }
    else
    {
        response = MakeCSResLogin(0, AccountNo);
        {
            SendPacket(player->sessionNo, response);
        }
        response->SubRef();
#endif
    }

    packet->SubRef();

    return true;
}

bool procademy::CChatServerSingle::RedisProc()
{
    while (1)
    {
        DWORD           transferredSize = 0;
        SESSION_ID      sessionNo;
        CNetPacket*     packet = nullptr;
        CNetPacket*     result = nullptr;
        char            buffer[12];
        INT64	        AccountNo;
        WCHAR	        ID[20];				// null 포함
        WCHAR	        Nickname[20];		// null 포함
        char	        SessionKey[65];		// 인증토큰
        st_MSG*         msg;
        bool            cmpRet = false;

        BOOL gqcsRet = GetQueuedCompletionStatus(mRedisIOCP, &transferredSize, (PULONG_PTR)&sessionNo, (LPOVERLAPPED*)&packet, INFINITE);

#ifdef PROFILE
        CProfiler::Begin(L"TokenProc");
        // Redis Thread End
        if (transferredSize == 0)
        {
            return true;
        }

        *packet >> AccountNo;

        packet->GetData(ID, 20);
        packet->GetData(Nickname, 20);
        packet->GetData(SessionKey, 64);
        SessionKey[64] = '\0';
        packet->SubRef();

        _i64toa_s(AccountNo, buffer, 12, 10);
        mRedis.get(buffer, [SessionKey, &cmpRet](cpp_redis::reply& reply) {
            if (reply.is_string())
            {
                cmpRet = strcmp(reply.as_string().c_str(), SessionKey) == 0;
            }
            });

        mRedis.sync_commit();

        result = MakeResultLogin(AccountNo, ID, Nickname);
        {
            msg = mMsgPool.Alloc();

            msg->sessionNo = sessionNo;

            result->AddRef();
            msg->packet = result;

            if (cmpRet)
            {
                msg->type = MSG_TYPE_VERIFICATION_SUCCESS;
            }
            else
            {
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"Redis Fail %d - %s", AccountNo, SessionKey);
                msg->type = MSG_TYPE_VERIFICATION_FAIL;
            }

            EnqueueMessage(msg);
        }
        result->SubRef();

        mRedisTPS++;
        CProfiler::End(L"TokenProc");
#else
        // Redis Thread End
        if (transferredSize == 0)
        {
            return true;
        }

        *packet >> AccountNo;

        packet->GetData(ID, 20);
        packet->GetData(Nickname, 20);
        packet->GetData(SessionKey, 64);
        SessionKey[64] = '\0';
        packet->SubRef();

        _i64toa_s(AccountNo, buffer, 12, 10);

        mRedis.sync_commit();

		mRedis.get(buffer, [SessionKey, &cmpRet](cpp_redis::reply& reply) {
            cmpRet = strcmp(reply.as_string().c_str(), SessionKey) == 0;
			});

        //mRedis.sync_commit();

        result = MakeResultLogin(AccountNo, ID, Nickname);
        {
            msg = mMsgPool.Alloc();

            msg->sessionNo = sessionNo;

            result->AddRef();
            msg->packet = result;

            if (cmpRet)
            {
                msg->type = MSG_TYPE_VERIFICATION_SUCCESS;
            }
            else
            {
                CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Redis Get Fail %lld - %s", AccountNo, SessionKey);
                msg->type = MSG_TYPE_VERIFICATION_FAIL;
            }

            EnqueueMessage(msg);
        }
        result->SubRef();

        mRedisTPS++;
#endif // PROFILE
    }

    return true;
}

void procademy::CChatServerSingle::BeginThreads()
{
    CProfiler::InitProfiler(30);

    mUpdateThread = (HANDLE)_beginthreadex(nullptr, 0, UpdateFunc, this, 0, nullptr);
    mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
    mHeartbeatThread = (HANDLE)_beginthreadex(nullptr, 0, HeartbeatFunc, this, 0, nullptr);
    mRedisThread = (HANDLE)_beginthreadex(nullptr, 0, RedisFunc, this, 0, nullptr);
}

procademy::st_Player* procademy::CChatServerSingle::FindPlayer(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    if (iter == mPlayerMap.end())
        return nullptr;
    
    return iter->second;
}

void procademy::CChatServerSingle::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    mPlayerMap[sessionNo] = player;
}

void procademy::CChatServerSingle::DeletePlayer(SESSION_ID sessionNo)
{
    mPlayerMap.erase(sessionNo);
}

void procademy::CChatServerSingle::Sector_AddPlayer(WORD x, WORD y, st_Player* player)
{
    mSector[y][x].list.push_back(player);
}

void procademy::CChatServerSingle::Sector_RemovePlayer(WORD x, WORD y, st_Player* player)
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
        mSector[curY][curX].playerCount += (DWORD)mSector[curY][curX].list.size();
        ret += (DWORD)mSector[curY][curX].list.size();

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

void procademy::CChatServerSingle::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Chat Server Status: %s]\n", mbBegin ? L"RUN" : L"STOP");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d] [SendToWorker: %d]\n", mbZeroCopy, mbNagle, mSendToWorker);
    idx += swprintf_s(s + idx, size - idx, L"[GQCS_EX: %d] [WorkerTh: %d] [ActiveTh: %d]\n", mGQCSEx, mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Player Num : ", mLoginCount);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    if (mGQCSEx == false)
        idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"MsgQ : ", mMsgQ.GetPoolCapacity(), mMsgQ.GetSize());
#ifdef TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Net Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"Player PoolAlloc Count : ", mPlayerPool.GetCapacity());
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"CNetChunk PoolAlloc Count : ", CNetPacket::sPacketPool.mMemoryPool->GetCapacity());
    idx += swprintf_s(s + idx, size - idx, L"%22s%d\n", L"MsgQ PoolAlloc Count : ", mMsgQ.mMemoryPool.GetCapacity());
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Lan Packet Pool : ", CLanPacket::sPacketPool.GetCapacity(), CLanPacket::sPacketPool.GetSize());
#endif // TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Update Msg Pool : ", mMsgPool.GetCapacity(), mMsgPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Player Pool : ", mPlayerPool.GetCapacity(), mPlayerPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Update TPS : ", mUpdateTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%.1f\n", L"GQCS_EX Avg : ", mUpdateTPS / (double)(mLoopCount == 0? 1 : mLoopCount));
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

void procademy::CChatServerSingle::MakeRatioMonitorStr(WCHAR* s, int size)
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

void procademy::CChatServerSingle::PrintRecvSendRatio()
{
    WCHAR* str = new WCHAR[MAX_STR];
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

                idx += swprintf_s(str + idx, MAX_STR - (LONGLONG)idx, L"%.2lf<%.2lf>,",
                    mSector[i][j].sendCount / (double)mSector[i][j].recvCount,
                    avgPlayers);
            }
            else
            {
                idx += swprintf_s(str + idx, MAX_STR - (LONGLONG)idx, L"%6d,", 0);
            }
        }

        idx += swprintf_s(str + idx, MAX_STR - (LONGLONG)idx, L"\n");
    }

    fwprintf_s(fout, str);

    fclose(fout);

    delete[] str;
}

void procademy::CChatServerSingle::ClearTPS()
{
    mUpdateTPS = 0;
    mRedisTPS = 0;
    mLoopCount = 0;
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

    mRatioMonitor.joinCount = 0;
    mRatioMonitor.loginCount = 0;
    mRatioMonitor.leaveCount = 0;
    mRatioMonitor.moveSectorCount = 0;
    mRatioMonitor.sendMsgInCount = 0;
    mRatioMonitor.sendMsgOutCount = 0;
}

void procademy::CChatServerSingle::Init()
{
    char IP[64];

    WideCharToMultiByte(CP_ACP, 0, mTokenDBIP, -1, IP, sizeof(IP), nullptr, nullptr);

    mUpdateEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);
    mRedis.connect(IP, mTokenDBPort);
    mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
    mRedisIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
}

void procademy::CChatServerSingle::RecordPerformence()
{
    CProfiler::SetRecord(L"Accept_TPS_AVG", (LONGLONG)mMonitor.acceptTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Update_TPS_AVG", (LONGLONG)mUpdateTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Recv_TPS_AVG", (LONGLONG)mMonitor.prevRecvTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"Send_TPS_AVG", (LONGLONG)mMonitor.prevSendTPS, CProfiler::PROFILE_TYPE::COUNT);
    CProfiler::SetRecord(L"CPU_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessorTotal(), CProfiler::PROFILE_TYPE::PERCENT);
    CProfiler::SetRecord(L"PROCESS_TOTAL_AVG", (LONGLONG)mCpuUsage.ProcessTotal(), CProfiler::PROFILE_TYPE::PERCENT);
}

void procademy::CChatServerSingle::LoginMonitorServer()
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

void procademy::CChatServerSingle::SendMonitorDataProc()
{
    CLanPacket* runPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN, mbBegin);

    mMonitorClient.SendPacket(runPacket);

    runPacket->SubRef();
    
    CLanPacket* cpuUsagePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU, (int)mCpuUsage.ProcessTotal());

    mMonitorClient.SendPacket(cpuUsagePacket);

    cpuUsagePacket->SubRef();

    CLanPacket* memoryUsagePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM, (int)mCpuUsage.ProcessUserMemory() / 1000000); // 1MB

    mMonitorClient.SendPacket(memoryUsagePacket);

    memoryUsagePacket->SubRef();

    CLanPacket* sessionNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_SESSION, (int)mPlayerMap.size());

    mMonitorClient.SendPacket(sessionNumPacket);

    sessionNumPacket->SubRef();

    CLanPacket* playerNumPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_PLAYER, (int)mLoginCount);

    mMonitorClient.SendPacket(playerNumPacket);

    playerNumPacket->SubRef();

    CLanPacket* updateTPSPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS, (int)mUpdateTPS);

    mMonitorClient.SendPacket(updateTPSPacket);

    updateTPSPacket->SubRef();

    CLanPacket* packetPoolSizePacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL, (int)CNetPacket::sPacketPool.GetSize());

    mMonitorClient.SendPacket(packetPoolSizePacket);

    packetPoolSizePacket->SubRef();

    CLanPacket* updateMSGPacket = MakeMonitorPacket(dfMONITOR_DATA_TYPE_CHAT_UPDATEMSG_POOL, (int)mMsgQ.GetSize());

    mMonitorClient.SendPacket(updateMSGPacket);

    updateMSGPacket->SubRef();

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

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResLogin(BYTE status, INT64 accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << accountNo;

    packet->SetHeader();
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << accountNo << sectorX << sectorY;

    packet->SetHeader();
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResMessage(INT64 accountNo, WCHAR* ID, WCHAR* nickname, WORD messageLen, WCHAR* message)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE << accountNo;

    packet->PutData(ID, 20);
    packet->PutData(nickname, 20);
    *packet << messageLen;
    packet->PutData(message, messageLen / 2);

    packet->SetHeader();
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeResultLogin(INT64 accountNo, WCHAR* ID, WCHAR* nickname)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << accountNo;

    packet->PutData(ID, 20);
    packet->PutData(nickname, 20);

    return packet;
}

procademy::CLanPacket* procademy::CChatServerSingle::MakeMonitorLogin(int serverNo)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_SS_MONITOR_LOGIN << serverNo;

    packet->SetHeader();

    return packet;
}

procademy::CLanPacket* procademy::CChatServerSingle::MakeMonitorPacket(BYTE dataType, int dataValue)
{
    CLanPacket* packet = CLanPacket::AllocAddRef();

    time_t timeval;

    time(&timeval);

    *packet << (WORD)en_PACKET_SS_MONITOR_DATA_UPDATE << dataType << dataValue << (int)timeval;

    packet->SetHeader();

    return packet;
}

void procademy::CChatServerSingle::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.LoadFile(fileName);

    tp.GetValue(L"MONITOR_SERVER_IP", mMonitorIP);

    tp.GetValue(L"MONITOR_SERVER_PORT", &num);
    mMonitorPort = (u_short)num;

    tp.GetValue(L"MONITOR_NO", &mServerNo);

#ifdef TLS_MEMORY_POOL_VER
    tp.GetValue(L"POOL_SIZE_CHECK", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
    {
        CNetPacket::sPacketPool.OnOffCounting();
        CLanPacket::sPacketPool.OnOffCounting();
        mMsgPool.OnOffCounting();
    }  
#endif // TLS_MEMORY_POOL_VER

    tp.GetValue(L"GQCSEX", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mGQCSEx = true;
    else
        mGQCSEx = false;
    
    tp.GetValue(L"REDIS_MODE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbRedisMode = true;
    else
        mbRedisMode = false;

    tp.GetValue(L"GQCSEX_NUM", &num);
    mGQCSCExNum = num;

    tp.GetValue(L"TIMEOUT_DISCONNECT", &mTimeOut);

    tp.GetValue(L"TOKEN_DB_IP", mTokenDBIP);
    tp.GetValue(L"TOKEN_DB_PORT", &num);
    mTokenDBPort = (USHORT)num;
}

void procademy::CChatServerSingle::FreePlayer(st_Player* player)
{
    player->accountNo = 0;
    player->lastRecvTime = 0;
    player->curSectorX = -1;
    player->curSectorY = -1;

    player->bLogin = false;

    mPlayerPool.Free(player);
}

