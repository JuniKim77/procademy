#include "PacketCreater.h"
#include "Protocol.h"
#include "CPacket.h"
#include "User.h"
#include <unordered_map>

using namespace std;

extern unordered_map<DWORD, User*> g_users;
extern unordered_map<DWORD, Room*> g_rooms;

void CreateResLoginPacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD userNo)
{
	*packet << result << userNo;

	FillHeader(header, packet, df_RES_LOGIN);
}

void CreateResRoomListPacket(st_PACKET_HEADER* header, CPacket* packet, DWORD roomNo)
{
	Room* room = g_rooms[roomNo];

	WORD size = (WORD)wcslen(room->mTitle);

	*packet << roomNo << size;

	packet->PutData(room->mTitle, size);

	*packet << (BYTE)room->mUserList.size();

	for (auto iter = room->mUserList.begin(); iter != room->mUserList.end(); ++iter)
	{
		packet->PutData(g_users[*iter]->GetName(), 15);
	}

	FillHeader(header, packet, df_RES_ROOM_LIST);
}

void CreateResRoomCreatePacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD roomNo)
{

}

BYTE makeCheckSum(CPacket* packet, WORD msgType)
{
	int size = packet->GetSize();
	BYTE* pBuf = (BYTE*)packet->GetBufferPtr();
	BYTE checkSum = 0;

	for (int i = 0; i < size; ++i)
	{
		checkSum += *pBuf;
		++pBuf;
	}

	return checkSum % 256;
}

void FillHeader(st_PACKET_HEADER* header, CPacket* packet, WORD msgType)
{
	header->byCode = dfPACKET_CODE;
	header->byCheckSum = makeCheckSum(packet, msgType);
	header->wMsgType = msgType;
	header->wPayloadSize = packet->GetSize();
}
