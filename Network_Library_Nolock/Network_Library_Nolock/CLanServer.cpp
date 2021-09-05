#include "CLanServer.h"
#include "CLogger.h"
#include "CPacket.h"

CLanServer::Session* CLanServer::FindSession(u_int64 sessionNo)
{
    return nullptr;
}

void CLanServer::InsertSessionData(u_int64 sessionNo, Session* session)
{
}

void CLanServer::DeleteSessionData(u_int64 sessionNo)
{
}

void CLanServer::UpdateSessionData(u_int64 sessionNo, Session* session)
{
}

bool CLanServer::CreateListenSocket()
{
    return false;
}

bool CLanServer::BeginThreads()
{
    return false;
}

unsigned int __stdcall CLanServer::WorkerThread(LPVOID arg)
{
    return 0;
}

unsigned int __stdcall CLanServer::AcceptThread(LPVOID arg)
{
    return 0;
}

bool CLanServer::RecvPost(Session* session)
{
    return false;
}

bool CLanServer::SendPost(Session* session)
{
    return false;
}

void CLanServer::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
{
}

bool CLanServer::DecrementProc(Session* session)
{
    return false;
}

void CLanServer::DisconnectProc(Session* session)
{
}

void CLanServer::PacketProc(Session* session, DWORD msgSize)
{
}

bool CLanServer::AcceptProc()
{
    return false;
}

Session* CLanServer::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
{
    return nullptr;
}

bool CLanServer::OnCompleteMessage()
{
    return false;
}

void CLanServer::CloseSessions()
{
}
