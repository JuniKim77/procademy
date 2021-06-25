#pragma once

#include <wtypes.h>
#include <list>

class CPacket;
struct st_PACKET_HEADER;

struct Room
{
	DWORD mRoomNo;
	WCHAR mTitle[256];
	std::list<DWORD> mUserList;
};

class User
{
public:
	User(DWORD userNo);
	User(DWORD userNo, const WCHAR* name);
	~User();
	void SetLogin() { mbLogin = true; }
	bool HasLogined() { return mbLogin; }
	WCHAR* GetName() { return mName; }

	bool PacketProc(WORD msgType, CPacket* packet);

private:
	bool ReqLogin(CPacket* packet);
	void ResLogin(BYTE result, DWORD to);
	bool ReqRoomList(CPacket* packet);
	void ResRoomList(DWORD roomNo, DWORD to);
	bool ReqRoomCreate(CPacket* packet);
	void ResRoomCreate(BYTE result, DWORD roomNo);
	bool ReqRoomEnter(CPacket* packet);
	bool ReqChat(CPacket* packet);
	bool ReqRoomLeave(CPacket* packet);

	void SendUnicast(DWORD to, st_PACKET_HEADER* header, CPacket* packet);
	void SendBroadcast(st_PACKET_HEADER* header, CPacket* packet);
	void SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet);

private:
	DWORD mUserNo;
	DWORD mRoomNo;
	WCHAR mName[15];
	bool mbLogin = false;
};