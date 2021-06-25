#include "Content.h"
#include "Protocol.h"
#include <stdio.h>
#include "CPacket.h"
#include <unordered_map>
#include "Session.h"
#include "User.h"
#include "PacketCreater.h"
#include "Container.h"

using namespace std;

extern DWORD g_SessionNo = 1;
extern DWORD g_RoomNo = 1;
extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet)
{
	wprintf_s(L"��Ŷ ���� [UserNo: %d][msgType: %d\n", from, msgType);

	switch (msgType)
	{
	case df_REQ_LOGIN:
		return ReqLogin(from, packet);
		break;
	case df_REQ_ROOM_LIST:
		return ReqRoomList(from, packet);
		break;
	case df_REQ_ROOM_CREATE:
		return ReqRoomCreate(from, packet);
		break;
	case df_REQ_ROOM_ENTER:
		return ReqRoomEnter(from, packet);
		break;
	case df_REQ_CHAT:
		return ReqChat(from, packet);
		break;
	case df_REQ_ROOM_LEAVE:
		return ReqRoomLeave(from, packet);
		break;
	default:
		break;
	}

	return true;
}

bool ReqLogin(DWORD from, CPacket* packet)
{
	WCHAR nickName[15];

	packet->GetData(nickName, _countof(nickName));

	// �ߺ� �˻�
	for (auto iter = g_users.begin(); iter != g_users.end(); ++iter)
	{
		if (wcscmp(iter->second->mName, nickName) == 0)
		{
			wprintf_s(L"�ߺ��� �̸�[%s] �Դϴ�.\n", nickName);
			ResLogin(from, df_RESULT_LOGIN_DNICK);

			g_sessions[from]->SetDisconnect();

			return false;
		}
	}

	wcscpy_s(g_users[from]->mName, _countof(g_users[from]->mName), nickName);
	g_users[from]->mbLogin = true;

	ResLogin(from, df_RESULT_LOGIN_OK);

	return true;
}

void ResLogin(DWORD to, BYTE result)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResLoginPacket(&sendHeader, &sendPacket, result, to);

	SendUnicast(to, &sendHeader, &sendPacket);
}

bool ReqRoomList(DWORD from, CPacket* packet)
{
	User* user = g_users[from];

	if (user->mRoomNo == 0)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �κ� �ֽ��ϴ�.\n", from, user->mName);

		return false;
	}

	ResRoomList(from, user->mRoomNo);

	return true;
}

void ResRoomList(DWORD to, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomListPacket(&sendHeader, &sendPacket, roomNo);

	SendUnicast(to, &sendHeader, &sendPacket);
}

bool ReqRoomCreate(DWORD client, CPacket* packet)
{
	User* user = g_users[client];

	if (user->mRoomNo != 0)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �κ� ���� �ʽ��ϴ�.\n", user->mUserNo, user->mName);
		ResRoomCreate(client, df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	WCHAR roomTitle[256] = { 0, };
	WORD titleSize;

	*packet >> titleSize;
	packet->GetData(roomTitle, titleSize);

	if (titleSize > sizeof(roomTitle))
	{
		wprintf_s(L"�� �̸� ���̰� 256�� �ʰ��մϴ�.\n");
		ResRoomCreate(client, df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}
	// �ߺ� ó��
	for (auto iter = g_rooms.begin(); iter != g_rooms.end(); ++iter)
	{
		if (wcscmp(iter->second->mTitle, roomTitle) == 0)
		{
			wprintf_s(L"�ߺ��� �� �̸�[%s] �Դϴ�.\n", roomTitle);
			ResRoomCreate(client, df_RESULT_ROOM_CREATE_DNICK, g_RoomNo);

			return false;
		}
	}


	ResRoomCreate(client, df_RESULT_ROOM_CREATE_OK, g_RoomNo);

	g_RoomNo++;

	return true;
}

void ResRoomCreate(DWORD client, BYTE result, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomCreatePacket(&sendHeader, &sendPacket, result, roomNo);

	SendBroadcast(&sendHeader, &sendPacket);
}

bool ReqRoomEnter(DWORD client, CPacket* packet)
{
	return false;
}

bool ReqChat(DWORD client, CPacket* packet)
{
	return false;
}

bool ReqRoomLeave(DWORD client, CPacket* packet)
{
	return false;
}

void SendUnicast(DWORD to, st_PACKET_HEADER* header, CPacket* packet)
{
	if (g_users[to] == nullptr)
	{
		wprintf_s(L"SendUnicast Dest Client is Null\n");
		return;
	}
	g_sessions[to]->sendPacket((char*)header, sizeof(st_PACKET_HEADER));
	g_sessions[to]->sendPacket(packet->GetBufferPtr(), packet->GetSize());
}

void SendBroadcast(st_PACKET_HEADER* header, CPacket* packet)
{
	for (auto iter = g_sessions.begin(); iter != g_sessions.end(); ++iter)
	{
		SendUnicast(iter->first, header, packet);
	}
}

void SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet)
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