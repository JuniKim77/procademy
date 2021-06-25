#include "User.h"
#include "CPacket.h"
#include "Protocol.h"
#include <unordered_map>
#include "Session.h"
#include <list>
#include "PacketCreater.h"

using namespace std;

extern DWORD g_RoomNo = 1;
extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

User::User(DWORD userNo)
	: mUserNo(userNo)
	, mRoomNo(0)
{
	memset(mName, 0, sizeof(mName));
}

User::User(DWORD userNo, const WCHAR* name)
	: mUserNo(userNo)
	, mRoomNo(0)
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
	WCHAR nickName[15];

	packet->GetData(nickName, _countof(nickName));

	// 중복 검사
	for (auto iter = g_users.begin(); iter != g_users.end(); ++iter)
	{
		if (wcscmp(iter->second->mName, nickName) == 0)
		{
			wprintf_s(L"중복된 이름[%s] 입니다.\n", nickName);
			ResLogin(df_RESULT_LOGIN_DNICK, mUserNo);

			g_sessions[mUserNo]->SetDisconnect();

			return false;
		}
	}

	wcscpy_s(mName, _countof(mName), nickName);
	SetLogin();

	ResLogin(df_RESULT_LOGIN_OK, mUserNo);

	return true;
}

void User::ResLogin(BYTE result, DWORD to)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResLoginPacket(&sendHeader, &sendPacket, result, to);

	SendUnicast(to, &sendHeader, &sendPacket);
}

bool User::ReqRoomList(CPacket* packet)
{
	if (!HasLogined())
	{
		wprintf_s(L"로그인하지 않은 유저입니다. [UserNo: %d]\n", mUserNo);

		return false;
	}

	if (mRoomNo == 0)
	{
		wprintf_s(L"해당 유저[%d, %s]는 로비에 있습니다.\n", mUserNo, mName);

		return false;
	}

	ResRoomList(mRoomNo, mUserNo);

	return true;
}

void User::ResRoomList(DWORD roomNo, DWORD to)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomListPacket(&sendHeader, &sendPacket, roomNo);
	
	SendUnicast(to, &sendHeader, &sendPacket);
}

bool User::ReqRoomCreate(CPacket* packet)
{
	if (!HasLogined())
	{
		wprintf_s(L"로그인하지 않은 유저입니다. [UserNo: %d]\n", mUserNo);
		ResRoomCreate(df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	if (mRoomNo != 0)
	{
		wprintf_s(L"해당 유저[%d, %s]는 로비에 있지 않습니다.\n", mUserNo, mName);
		ResRoomCreate(df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	WCHAR roomTitle[256] = { 0, };
	WORD titleSize;

	*packet >> titleSize;
	packet->GetData(roomTitle, titleSize);

	if (titleSize > sizeof(roomTitle))
	{
		wprintf_s(L"방 이름 길이가 256를 초과합니다.\n");
		ResRoomCreate(df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}
	// 중복 처리
	for (auto iter = g_rooms.begin(); iter != g_rooms.end(); ++iter)
	{
		if (wcscmp(iter->second->mTitle, roomTitle) == 0)
		{
			wprintf_s(L"중복된 방 이름[%s] 입니다.\n", roomTitle);
			ResRoomCreate(df_RESULT_ROOM_CREATE_DNICK, g_RoomNo);

			return false;
		}
	}


	ResRoomCreate(df_RESULT_ROOM_CREATE_OK, g_RoomNo);

	g_RoomNo++;

	return true;
}

void User::ResRoomCreate(BYTE result, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomCreatePacket(&sendHeader, &sendPacket, result, roomNo);

	SendBroadcast(&sendHeader, &sendPacket);
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

void User::SendUnicast(DWORD to, st_PACKET_HEADER* header, CPacket* packet)
{
	if (g_users[to] == nullptr)
	{
		wprintf_s(L"SendUnicast Dest Client is Null\n");
		return;
	}
	g_sessions[to]->sendPacket((char*)header, sizeof(st_PACKET_HEADER));
	g_sessions[to]->sendPacket(packet->GetBufferPtr(), packet->GetSize());
}

void User::SendBroadcast(st_PACKET_HEADER* header, CPacket* packet)
{
	for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter)
	{
		SendUnicast(iter->first, header, packet);
	}
}

void User::SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet)
{
	for (auto iter = g_rooms[roomNo]->mUserList.begin();
		iter != g_rooms[roomNo]->mUserList.end(); ++iter)
	{
		DWORD userNo = *iter;

		if (userNo != from)
		{
			SendUnicast(userNo, header, packet);
		}
	}
}
