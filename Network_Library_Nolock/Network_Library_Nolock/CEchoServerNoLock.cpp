#include "CEchoServerNoLock.h"
#include "MessageProtocol.h"
#include "CPacket.h"

bool CEchoServerNoLock::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void CEchoServerNoLock::OnClientJoin(SESSION_ID SessionID)
{
	InsertSessionID(SessionID);
}

void CEchoServerNoLock::OnClientLeave(SESSION_ID SessionID)
{
	DeleteSessionID(SessionID);
}

void CEchoServerNoLock::OnRecv(SESSION_ID SessionID, CPacket* packet)
{
	//CompletePacket(SessionID, packet);

	SendPacket(SessionID, packet);
}

void CEchoServerNoLock::OnError(int errorcode, WCHAR* log)
{
}

void CEchoServerNoLock::InsertSessionID(u_int64 sessionNo)
{
	mSessionSet.insert(sessionNo);
}

void CEchoServerNoLock::DeleteSessionID(u_int64 sessionNo)
{
	auto iter = mSessionSet.find(sessionNo);

	if (iter == mSessionSet.end())
	{
		return;
	}

	mSessionSet.erase(iter);
}

void CEchoServerNoLock::CompletePacket(SESSION_ID SessionID, CPacket* packet)
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

void CEchoServerNoLock::EchoProc(SESSION_ID sessionID, CPacket* packet)
{
	SendPacket(sessionID, packet);
}