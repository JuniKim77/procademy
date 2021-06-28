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

extern DWORD g_SessionNo;
extern DWORD g_RoomNo;
extern unordered_map<DWORD, Session*> g_sessions;
extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet)
{
	wprintf_s(L"��Ŷ ���� [UserNo: %d][msgType: %d]\n", from, msgType);

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
	Session* fromSession = FindSession(from);

	packet->GetData(nickName, dfNICK_MAX_LEN);

	// �ߺ� �˻�
	for (auto iter = g_users.begin(); iter != g_users.end(); ++iter)
	{
		if (wcscmp(iter->second->mName, nickName) == 0)
		{
			wprintf_s(L"�ߺ��� �̸�[%s] �Դϴ�.\n", nickName);
			ResLogin(from, df_RESULT_LOGIN_DNICK);

			fromSession->SetDisconnect();

			return false;
		}
	}

	User* user = FindUser(from);

	wcscpy_s(user->mName, _countof(user->mName), nickName);
	user->mbLogin = true;

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
	User* user = FindUser(from);

	if (user->mRoomNo != 0)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �κ� ���� �ʽ��ϴ�.\n", from, user->mName);

		return false;
	}

	ResRoomList(from);

	return true;
}

void ResRoomList(DWORD to)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomListPacket(&sendHeader, &sendPacket);

	SendUnicast(to, &sendHeader, &sendPacket);
}

bool ReqRoomCreate(DWORD client, CPacket* packet)
{
	User* user = FindUser(client);

	if (user->mbLogin == false)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �α������� �ʾҽ��ϴ�.\n", user->mUserNo, user->mName);
		ResRoomCreate(client, df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	if (user->mRoomNo != 0)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �κ� ���� �ʽ��ϴ�.\n", user->mUserNo, user->mName);
		ResRoomCreate(client, df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	WCHAR roomTitle[256] = { 0, };
	WORD titleSize;

	*packet >> titleSize;
	packet->GetData(roomTitle, titleSize / sizeof(WCHAR));

	if (titleSize >= sizeof(roomTitle))
	{
		wprintf_s(L"�� �̸� ���̰� 256�� �ʰ��մϴ�.\n");
		ResRoomCreate(client, df_RESULT_ROOM_CREATE_ETC, g_RoomNo);

		return false;
	}

	// �ߺ� ó��
	for (auto iter = g_rooms.begin(); iter != g_rooms.end(); ++iter)
	{
		if (iter->second == nullptr)
			continue;

		if (wcscmp(iter->second->mTitle, roomTitle) == 0)
		{
			wprintf_s(L"�ߺ��� �� �̸�[%s] �Դϴ�.\n", roomTitle);
			ResRoomCreate(client, df_RESULT_ROOM_CREATE_DNICK, g_RoomNo);

			return false;
		}
	}

	Room* room = new Room;
	room->mRoomNo = g_RoomNo;
	wcscpy_s(room->mTitle, _countof(room->mTitle), roomTitle);

	InsertRoomData(g_RoomNo, room);

	wprintf_s(L"�� ���� [UserNo:%d][Room:%s] [TotalRoom:%d]\n",
		client, roomTitle, g_rooms.size());

	ResRoomCreate(client, df_RESULT_ROOM_CREATE_OK, g_RoomNo);

	g_RoomNo++;

	return true;
}

void ResRoomCreate(DWORD client, BYTE result, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomCreatePacket(&sendHeader, &sendPacket, result, roomNo);

	if (result == df_RESULT_ROOM_CREATE_OK)
	{
		SendBroadcast(&sendHeader, &sendPacket);
	}
	else
	{
		SendUnicast(client, &sendHeader, &sendPacket);
	}
}

bool ReqRoomEnter(DWORD client, CPacket* packet)
{
	User* user = FindUser(client);
	DWORD roomNo;
	*packet >> roomNo;

	if (user->mbLogin == false)
	{
		wprintf_s(L"�ش� ����[%d, %s]�� �α������� �ʾҽ��ϴ�.\n", user->mUserNo, user->mName);
		ResRoomEnter(client, df_RESULT_ROOM_ENTER_ETC, roomNo);

		return false;
	}

	for (auto iter = g_rooms.begin(); iter != g_rooms.end(); ++iter)
	{
		if (iter->second == nullptr)
			continue;

		if (iter->second->mRoomNo == roomNo)
		{
			ResRoomOtherUserEnter(client, roomNo);

			InsertUserToRoom(client, iter->second);

			ResRoomEnter(client, df_RESULT_ROOM_ENTER_OK, roomNo);

			return true;
		}
	}

	ResRoomEnter(client, df_RESULT_ROOM_ENTER_NOT, roomNo);

	return false;
}

void ResRoomEnter(DWORD from, BYTE result, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomEnterPacket(&sendHeader, &sendPacket, result, roomNo);

	SendUnicast(from, &sendHeader, &sendPacket);
}

void ResRoomOtherUserEnter(DWORD client, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	User* user = FindUser(client);

	CreateResOtherUserRoomEnterPacket(&sendHeader, &sendPacket, user->mName, client);

	SendBroadcast_room(roomNo, client, &sendHeader, &sendPacket);
}

bool ReqChat(DWORD client, CPacket* packet)
{
	User* user = FindUser(client);

	if (user->mRoomNo == 0)
	{
		return false;
	}

	WORD size;
	*packet >> size;
	WCHAR msg[512];

	packet->GetData(msg, size);
	msg[size] = L'\0';

	ResChat(client, user->mRoomNo, size, msg);

	return true;
}

void ResChat(DWORD from, DWORD roomNo, WORD msgSize, const WCHAR* msg)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResChatPacket(&sendHeader, &sendPacket, from, msgSize, msg);

	SendBroadcast_room(roomNo, from, &sendHeader, &sendPacket);
}

bool ReqRoomLeave(DWORD client, CPacket* packet)
{
	User* user = FindUser(client);
	Room* room = FindRoom(user->mRoomNo);

	ResRoomLeave(client, user->mRoomNo);

	for (auto iter = room->mUserList.begin(); iter != room->mUserList.end();)
	{
		if (*iter == client)
		{
			room->mUserList.erase(iter);
			break;
		}
		else
		{
			++iter;
		}
	}

	if (room->mUserList.empty())
	{
		ResRoomDelete(user->mRoomNo);

		DeleteRoomData(user->mRoomNo);
	}

	user->mRoomNo = 0;

	return true;
}

void ResRoomLeave(DWORD client, DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomLeavePacket(&sendHeader, &sendPacket, client);

	SendUnicast(client, &sendHeader, &sendPacket);

	SendBroadcast_room(roomNo, client, &sendHeader, &sendPacket);
}

void ResRoomDelete(DWORD roomNo)
{
	CPacket sendPacket;
	st_PACKET_HEADER sendHeader;

	CreateResRoomDeletePacket(&sendHeader, &sendPacket, roomNo);

	SendBroadcast(&sendHeader, &sendPacket);
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
		if (iter->second == nullptr)
			continue;

		SendUnicast(iter->first, header, packet);
	}
}

void SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet)
{
	Room* room = FindRoom(roomNo);

	for (auto iter = room->mUserList.begin();
		iter != room->mUserList.end(); ++iter)
	{
		DWORD userNo = *iter;

		if (userNo != from)
		{
			SendUnicast(userNo, header, packet);
		}
	}
}