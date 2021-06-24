#include "User.h"
#include "CPacket.h"
#include "Protocol.h"
#include <unordered_map>
#include "Session.h"
#include <list>

using namespace std;

extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

User::User(DWORD userNo, const WCHAR* name)
	: mUserNo(userNo)
{
	wcscpy_s(mName, _countof(mName), name);

	mName[_countof(mName) - 1] = L'\0';
}

User::~User()
{
}

bool User::PacketProc(WORD msgType, CPacket* packet)
{
	switch (msgType)
	{
	case df_REQ_LOGIN:
		return ReqLogin(packet);
		break;
	case df_REQ_ROOM_LIST:
		return ReqRoomList(packet);
		break;
	case df_REQ_ROOM_CREATE:
		return ReqRoomCreate(packet);
		break;
	case df_REQ_ROOM_ENTER:
		return ReqRoomEnter(packet);
		break;
	case df_REQ_CHAT:
		return ReqChat(packet);
		break;
	case df_REQ_ROOM_LEAVE:
		return ReqRoomLeave(packet);
		break;
	default:
		break;
	}

	return true;
}

bool User::ReqLogin(CPacket* packet)
{
	return false;
}

bool User::ReqRoomList(CPacket* packet)
{
	return false;
}

bool User::ReqRoomCreate(CPacket* packet)
{
	return false;
}

bool User::ReqRoomEnter(CPacket* packet)
{
	return false;
}

bool User::ReqChat(CPacket* packet)
{
	return false;
}

bool User::ReqRoomLeave(CPacket* packet)
{
	return false;
}

void User::SendUnicast(DWORD to, CPacket* packet)
{
	if (g_users[to] == nullptr)
	{
		wprintf_s(L"SendUnicast Dest Client is Null\n");
		return;
	}

	g_sessions[to]->sendPacket(packet->GetBufferPtr(), packet->GetSize());
}

void User::SendBroadcast(CPacket* packet)
{
	for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter)
	{
		SendUnicast(iter->first, packet);
	}
}

void User::SendBroadcast_room(DWORD roomNo, DWORD from, CPacket* packet)
{
	for (auto iter = g_rooms[roomNo]->mUserList.begin();
		iter != g_rooms[roomNo]->mUserList.end(); ++iter)
	{
		DWORD userNo = *iter;

		if (userNo != from)
		{
			SendUnicast(userNo, packet);
		}
	}
}
