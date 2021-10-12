#include "CEchoServerNoLock.h"
#include "CCrashDump.h"
#include "CPacket.h"
#include "MessageProtocol.h"

CEchoServerNoLock::CEchoServerNoLock()
{
	InitializeSRWLock(&mSetLock);
}

bool CEchoServerNoLock::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void CEchoServerNoLock::OnClientJoin(SESSION_ID SessionID)
{
	CPacket* packet = new CPacket;

	WORD len = 8;
	int64_t value = 0x7fffffffffffffff;

	*packet << len << value;

	SendPacket(SessionID, packet);
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

void CEchoServerNoLock::OnError(int errorcode, const WCHAR* log)
{
}

void CEchoServerNoLock::InsertSessionID(u_int64 sessionNo)
{
	LockMap();
	mSessionJoinMap[sessionNo]++;
	if (mSessionJoinMap[sessionNo] > 1)
	{
		CRASH();
	}
	UnlockMap();
}

void CEchoServerNoLock::DeleteSessionID(u_int64 sessionNo)
{
	LockMap();
	mSessionJoinMap[sessionNo]--;
	if (mSessionJoinMap[sessionNo] == 0)
	{
		mSessionJoinMap.erase(sessionNo);
	}
	else
	{
		CRASH();
	}
	UnlockMap();
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

void CEchoServerNoLock::LockMap()
{
}

void CEchoServerNoLock::UnlockMap()
{
}
