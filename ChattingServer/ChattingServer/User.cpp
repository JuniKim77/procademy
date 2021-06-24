#include "User.h"
#include "CPacket.h"
#include "Protocol.h"

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
