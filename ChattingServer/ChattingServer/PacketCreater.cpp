#include "PacketCreater.h"
#include "Protocol.h"
#include "CPacket.h"
#include "User.h"
#include <unordered_map>
#include "Container.h"

using namespace std;

extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

void CreateResLoginPacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD userNo)
{
	*packet << result << userNo;

	FillHeader(header, packet, df_RES_LOGIN);
}

void CreateResRoomListPacket(st_PACKET_HEADER* header, CPacket* packet)
{
	*packet << (WORD)g_rooms.size();

	for (auto iter = g_rooms.begin(); iter != g_rooms.end(); ++iter)
	{
		Room* room = iter->second;

		if (room == nullptr)
		{
			continue;
		}

		*packet << room->mRoomNo;

		WORD size = wcslen(room->mTitle) * sizeof(WCHAR);
		*packet << size;
		packet->PutData(room->mTitle, size / sizeof(WCHAR));

		*packet << (BYTE)room->mUserList.size();
		for (auto iter = room->mUserList.begin(); iter != room->mUserList.end(); ++iter)
		{
			User* user = FindUser(*iter);

			packet->PutData(user->mName, dfNICK_MAX_LEN);
		}
	}

	FillHeader(header, packet, df_RES_ROOM_LIST);
}

void CreateResRoomCreatePacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD roomNo)
{
	*packet << result;

	if (result == df_RESULT_ROOM_CREATE_OK)
	{
		Room* room = FindRoom(roomNo);

		*packet << roomNo;
		WORD size = wcslen(room->mTitle) * sizeof(WCHAR);

		*packet << size;
		packet->PutData(room->mTitle, size / sizeof(WCHAR));
	}

	FillHeader(header, packet, df_RES_ROOM_CREATE);
}

void CreateResRoomEnterPacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD roomNo)
{
	*packet << result;

	if (result == df_RESULT_ROOM_ENTER_OK)
	{
		*packet << roomNo;
		Room* room = FindRoom(roomNo);

		WORD size = wcslen(room->mTitle) * sizeof(WCHAR);

		*packet << size;
		packet->PutData(room->mTitle, size / sizeof(WCHAR));

		*packet << (BYTE)room->mUserList.size();

		for (auto iter = room->mUserList.begin(); iter != room->mUserList.end(); ++iter)
		{
			User* user = FindUser(*iter);

			packet->PutData(user->mName, dfNICK_MAX_LEN);
			*packet << user->mUserNo;
		}
	}

	FillHeader(header, packet, df_RES_ROOM_ENTER);
}

void CreateResChatPacket(st_PACKET_HEADER* header, CPacket* packet, DWORD from, WORD msgSize, const WCHAR* msg)
{
	*packet << from << msgSize;
	packet->PutData(msg, msgSize);

	FillHeader(header, packet, df_RES_CHAT);
}

void CreateResRoomLeavePacket(st_PACKET_HEADER* header, CPacket* packet, DWORD from)
{
	*packet << from;

	FillHeader(header, packet, df_RES_ROOM_LEAVE);
}

void CreateResRoomDeletePacket(st_PACKET_HEADER* header, CPacket* packet, DWORD roomNo)
{
	*packet << roomNo;

	FillHeader(header, packet, df_RES_ROOM_DELETE);
}

void CreateResOtherUserRoomEnterPacket(st_PACKET_HEADER* header, CPacket* packet, const WCHAR* name, DWORD userNo)
{
	packet->PutData(name, dfNICK_MAX_LEN);
	*packet << userNo;

	FillHeader(header, packet, df_RES_USER_ENTER);
}

BYTE makeCheckSum(CPacket* packet, WORD msgType)
{
	int size = packet->GetSize();
	BYTE* pBuf = (BYTE*)packet->GetBufferPtr();
	int checkSum = msgType;

	for (int i = 0; i < size; ++i)
	{
		checkSum += *pBuf;
		++pBuf;
	}

	return (BYTE)(checkSum % 256);
}

void FillHeader(st_PACKET_HEADER* header, CPacket* packet, WORD msgType)
{
	header->byCode = dfPACKET_CODE;
	header->byCheckSum = makeCheckSum(packet, msgType);
	header->wMsgType = msgType;
	header->wPayloadSize = packet->GetSize();
}
