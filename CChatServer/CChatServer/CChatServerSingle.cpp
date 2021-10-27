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

void procademy::CChatServerSingle::Initialize()
{
}

void procademy::CChatServerSingle::EnqueueMessage(st_MSG* packet)
{
}

bool procademy::CChatServerSingle::GQCSProc()
{
    DWORD transferredSize = 0;
    CNetPacket* pPacket = nullptr;
    WSAOVERLAPPED* pOverlapped = nullptr;
    
    BOOL gqcsRet = GetQueuedCompletionStatus(mIOCP, &transferredSize, (PULONG_PTR)&pPacket, &pOverlapped, INFINITE);



    return false;
}

bool procademy::CChatServerSingle::CheckHeart()
{
    return false;
}

bool procademy::CChatServerSingle::LoginProc(u_int64 sessionNo)
{
    return false;
}

bool procademy::CChatServerSingle::MoveSectorProc(u_int64 sessionNo)
{
    return false;
}

bool procademy::CChatServerSingle::SendMessageProc(u_int64 sessionNo)
{
    return false;
}

procademy::CChatServerSingle::st_Player* procademy::CChatServerSingle::FindPlayer(u_int64 sessionNo)
{
    return nullptr;
}

void procademy::CChatServerSingle::InsertPlayer(st_Player* player)
{
}

void procademy::CChatServerSingle::DeletePlayer(u_int64 sessionNo)
{
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResLogin(BYTE status, INT64 accountNo)
{
    return nullptr;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResSectorMove(INT64 accountNo, WORD sectorX, WORD sectorY)
{
    return nullptr;
}

procademy::CNetPacket* procademy::CChatServerSingle::MakeCSResMessage(INT64 accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message)
{
    return nullptr;
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
}

void procademy::CChatServerSingle::OnClientLeave(SESSION_ID SessionID)
{
}

void procademy::CChatServerSingle::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{
    st_MSG* msg;
    WORD type;

    *packet >> type;

    switch (type)
    {
    case en_PACKET_CS_CHAT_REQ_LOGIN:
        msg = mMsgPool.Alloc();

        break;
    case en_PACKET_CS_CHAT_REQ_SECTOR_MOVE:
        break;
    case en_PACKET_CS_CHAT_REQ_MESSAGE:
        break;
    case en_PACKET_CS_CHAT_REQ_HEARTBEAT:
        break;
    default:
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"OnRecv: Undefined Req type [Session: %llu]\n", SessionID);
        break;
    }
}

void procademy::CChatServerSingle::OnError(int errorcode, const WCHAR* log)
{
}
