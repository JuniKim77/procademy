#include "CChatServerSingle.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"

unsigned int __stdcall procademy::CChatServerSingle::UpdateFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    while (1)
    {
        if (!chatServer->GQCSProc())
        {
            break;
        }
    }

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::MonitorFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->MonitoringProc();

    return 0;
}

unsigned int __stdcall procademy::CChatServerSingle::HeartbeatFunc(LPVOID arg)
{
    CChatServerSingle* chatServer = (CChatServerSingle*)arg;

    chatServer->CheckHeart();

    return 0;
}

void procademy::CChatServerSingle::Initialize()
{
}

void procademy::CChatServerSingle::EnqueueMessage(st_MSG* packet)
{
    PostQueuedCompletionStatus(mIOCP, 0, (ULONG_PTR)packet, 0);
}

bool procademy::CChatServerSingle::GQCSProc()
{
    DWORD transferredSize = 0;
    st_MSG* completionKey = nullptr;
    WSAOVERLAPPED* pOverlapped = nullptr;
    
    BOOL gqcsRet = GetQueuedCompletionStatus(mIOCP, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);

    // ECHO Server End
    if (completionKey == nullptr)
    {
        return false;
    }

    switch (completionKey->type)
    {
	case MSG_TYPE_RECV:
        CompleteMessage(completionKey->sessionNo, completionKey->packet);
		break;
	case MSG_TYPE_CONNECT:
		break;
	case MSG_TYPE_DISCONNECT:
		break;
	case MSG_TYPE_TIMEOUT:
		break;
	default:
		break;
	}


    return true;
}

bool procademy::CChatServerSingle::CheckHeart()
{
    HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);

    while (1)
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

    while (1)
    {
        DWORD retval = WaitForSingleObject(dummyevent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            // 출력
        }
    }

    CloseHandle(dummyevent);

    return true;
}

bool procademy::CChatServerSingle::CompleteMessage(u_int64 sessionNo, CNetPacket* packet)
{
    BYTE type;

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
        break;
    default:
        break;
    }

    packet->SubRef();

    return false;
}

bool procademy::CChatServerSingle::LoginProc(u_int64 sessionNo, CNetPacket* packet)
{
	INT64	    AccountNo;
	WCHAR	    ID[20];				// null 포함
	WCHAR	    Nickname[20];		// null 포함
	char	    SessionKey[64];		// 인증토큰
    CNetPacket* response;
	st_Player*  player = FindPlayer(sessionNo);

    if (player == nullptr)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Player[%llu] Not Found\n", sessionNo);

        response = MakeCSResLogin(0, 0);
        SendPacket(sessionNo, response);

        return false;
    }

    *packet >> AccountNo;

    packet->GetData(ID, 20);
    packet->GetData(Nickname, 20);
    packet->GetData(SessionKey, 64);

    // token verification

    player->accountNo = AccountNo;
    memcpy_s(player->ID, sizeof(player->ID), ID, sizeof(ID));
    memcpy_s(player->nickName, sizeof(player->nickName), Nickname, sizeof(Nickname));
    player->bLogin = true;
    player->lastRecvTime = GetTickCount64();

    response = MakeCSResLogin(1, player->accountNo);

    SendPacket(sessionNo, response);
    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::MoveSectorProc(u_int64 sessionNo, CNetPacket* packet)
{
	INT64	AccountNo;
	WORD	SectorX;
	WORD	SectorY;
    st_Player* player = FindPlayer(sessionNo);

    *packet >> AccountNo >> SectorX >> SectorY;

    if (player->accountNo != AccountNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched\n",
            sessionNo, player->accountNo, AccountNo);

        return false;
    }

    if (SectorX < 0 || SectorY < 0 || SectorX >= SECTOR_MAX_X || SectorY >= SECTOR_MAX_Y)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Move Sector - [Session %llu] [AccountNo %lld] Out of Boundary\n",
            sessionNo, AccountNo);

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

    SendPacket(sessionNo, response);
    response->SubRef();

    return true;
}

bool procademy::CChatServerSingle::SendMessageProc(u_int64 sessionNo, CNetPacket* packet)
{
    INT64	            AccountNo;
    WORD                messageLen;
    st_Player*          player = FindPlayer(sessionNo);
    st_Sector_Around    sectorAround;

    *packet >> AccountNo >> messageLen;

    if (player->accountNo != AccountNo)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendMessageProc - [Session %llu] [pAccountNo %lld] [AccountNo %lld] Not Matched\n",
            sessionNo, player->accountNo, AccountNo);

        return false;
    }

    GetSectorAround(player->curSectorX, player->curSectorY, &sectorAround);

    SendMessageSectorAround(packet, messageLen, &sectorAround);

    return true;
}

procademy::CChatServerSingle::st_Player* procademy::CChatServerSingle::FindPlayer(u_int64 sessionNo)
{
    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    if (iter == mPlayerMap.end())
        return nullptr;
    else
        return iter->second;
}

void procademy::CChatServerSingle::InsertPlayer(u_int64 sessionNo, st_Player* player)
{
    mPlayerMap[sessionNo] = player;
}

void procademy::CChatServerSingle::DeletePlayer(u_int64 sessionNo)
{
    mPlayerMap.erase(sessionNo);
}

void procademy::CChatServerSingle::Sector_AddPlayer(WORD x, WORD y, st_Player* player)
{
    mSector[y][x].push_back(player);
}

void procademy::CChatServerSingle::Sector_RemovePlayer(WORD x, WORD y, st_Player* player)
{
    for (auto iter = mSector[y][x].begin(); iter != mSector[y][x].end(); ++iter)
    {
        if ((*iter)->accountNo == player->accountNo)
        {
            mSector[y][x].erase(iter);

            return;
        }
    }

    CLogger::_Log(dfLOG_LEVEL_ERROR, L"DeleteFromSector[X %u][Y %u] - Not Found Player[%lld]",
        x, y, player->accountNo);
}

void procademy::CChatServerSingle::GetSectorAround(WORD x, WORD y, st_Sector_Around* output)
{
    WORD beginX = x == 0 ? 0 : x - 1;
    WORD beginY = y == 0 ? 0 : y - 1;

    output->count = 0;

    for (WORD i = beginY; i < y + 2; ++i)
    {
        for (WORD j = beginX; j < x + 2; ++j)
        {
            output->around[output->count].y = j;
            output->around[output->count].x = i;
            output->count++;
        }
    }
}

void procademy::CChatServerSingle::SendMessageSectorAround(CNetPacket* packet, WORD size, st_Sector_Around* input)
{
    
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResLogin(BYTE status, INT64 accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << accountNo;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << accountNo << sectorX << sectorY;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResMessage(INT64 accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE << accountNo;

    packet->PutData(ID, 20);
    packet->PutData(nickname, 20);
    *packet << meesageLen;
    packet->PutData(message, meesageLen);

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CChatServerSingle::CChatServerSingle()
{
}

procademy::CChatServerSingle::~CChatServerSingle()
{
}

bool procademy::CChatServerSingle::OnConnectionRequest(u_long IP, u_short Port)
{
    return false;
}

void procademy::CChatServerSingle::OnClientJoin(SESSION_ID SessionID)
{
    st_MSG* msg;

    msg = mMsgPool.Alloc();

    msg->type = MSG_TYPE_CONNECT;
    msg->sessionNo = SessionID;
    msg->packet = nullptr;

    EnqueueMessage(msg);
}

void procademy::CChatServerSingle::OnClientLeave(SESSION_ID SessionID)
{
    st_MSG* msg;

    msg = mMsgPool.Alloc();

    msg->type = MSG_TYPE_DISCONNECT;
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

bool procademy::CChatServerSingle::BeginServer(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
    if (Start(port, ip, createThread, runThread, nagle, maxClient) == false)
    {
        return false;
    }

    WaitForThreadsFin();

    return true;
}

bool procademy::CChatServerSingle::BeginServer(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
    return BeginServer(port, INADDR_ANY, createThread, runThread, nagle, maxClient);
}
