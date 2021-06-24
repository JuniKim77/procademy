#pragma once

#include <wtypes.h>
#include <list>

class CPacket;

struct Room
{
	DWORD mRoomNo;
	WCHAR mTitle[256];
	std::list<DWORD> mUserList;
};

class User
{
public:
	User(DWORD userNo, const WCHAR* name);
	~User();

	bool PacketProc(WORD msgType, CPacket* packet);

private:
	bool ReqLogin(CPacket* packet);
	bool ReqRoomList(CPacket* packet);
	bool ReqRoomCreate(CPacket* packet);
	bool ReqRoomEnter(CPacket* packet);
	bool ReqChat(CPacket* packet);
	bool ReqRoomLeave(CPacket* packet);

	void SendUnicast(DWORD to, CPacket* packet);
	void SendBroadcast(CPacket* packet);
	void SendBroadcast_room(DWORD roomNo, DWORD from, CPacket* packet);

private:
	DWORD mUserNo;
	DWORD mRoomNo;
	WCHAR mName[15];
};