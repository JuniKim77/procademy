#include "CEchoServer.h"
#include "MessageProtocol.h"
#include "CPacket.h"

bool CEchoServer::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void CEchoServer::OnClientJoin(SESSION_ID SessionID)
{
}

void CEchoServer::OnClientLeave(SESSION_ID SessionID)
{
}

void CEchoServer::OnRecv(SESSION_ID SessionID, CPacket* packet)
{
	CompletePacket(SessionID, packet);
}

void CEchoServer::OnError(int errorcode, WCHAR* log)
{
}

void CEchoServer::InsertSessionID(u_int64 sessionNo)
{
	mSessionSet.insert(sessionNo);
}

void CEchoServer::DeleteSessionID(u_int64 sessionNo)
{
	auto iter = mSessionSet.find(sessionNo);

	if (iter == mSessionSet.end())
	{
		return;
	}

	mSessionSet.erase(iter);
}

void CEchoServer::CompletePacket(SESSION_ID SessionID, CPacket* packet)
{
	st_MESSAGE_HEADER* header = (st_MESSAGE_HEADER*)packet->GetBufferPtr();

	switch (header->wMessageType)
	{
	case dfMESSAGE_ECHO:
		EchoProc(SessionID, packet);
		break;
	default:
		break;
	}
}

void CEchoServer::EchoProc(SESSION_ID sessionID, CPacket* packet)
{
	SendPacket(sessionID, packet);
}
