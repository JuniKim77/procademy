#include "CEchoServer.h"
#include "MessageProtocol.h"
#include "CPacket.h"

bool CEchoServer::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void CEchoServer::OnClientJoin(SESSION_ID SessionID)
{
	InsertSessionID(SessionID);
}

void CEchoServer::OnClientLeave(SESSION_ID SessionID)
{
	DeleteSessionID(SessionID);
}

void CEchoServer::OnRecv(SESSION_ID SessionID, CPacket* packet)
{
	//CompletePacket(SessionID, packet);

	SendPacket(SessionID, packet);
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
	/*st_MESSAGE_HEADER* header = (st_MESSAGE_HEADER*)packet->GetBufferPtr();

	switch (header->wMessageType)
	{
	default:
		EchoProc(SessionID, packet);
		break;
	}*/

	CPacket msg;

	while (packet->GetSize() > 0)
	{
		st_MESSAGE_HEADER header;

		*packet >> header.wPayloadSize;

		msg.PutData((char*)&header.wPayloadSize, sizeof(header));

		msg.PutData(packet->GetFrontPtr(), header.wPayloadSize);

		packet->MoveFront(header.wPayloadSize);

		SendPacket(SessionID, &msg);

		msg.Clear();
	}
}

void CEchoServer::EchoProc(SESSION_ID sessionID, CPacket* packet)
{
	SendPacket(sessionID, packet);
}
