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

struct User
{
	DWORD mUserNo;
	DWORD mRoomNo;
	WCHAR mName[15];
	bool mbLogin = false;

	User(DWORD userNo)
		: mUserNo(userNo)
		, mRoomNo(0)
	{
		memset(mName, 0, sizeof(mName));
	}

	User(DWORD userNo, const WCHAR* name)
		: mUserNo(userNo)
		, mRoomNo(0)
	{
		wcscpy_s(mName, _countof(mName), name);

		mName[_countof(mName) - 1] = L'\0';
	}
};