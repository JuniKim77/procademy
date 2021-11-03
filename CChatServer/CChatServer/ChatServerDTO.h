#pragma once

namespace procademy
{
	class st_Player;

	struct st_Sector
	{
		std::list<st_Player*>	list;
		DWORD					recvCount;
		DWORD					sendCount;
		DWORD					playerCount;
		DWORD					updateCount;
	};

	struct st_Coordinate
	{
		int		x;
		int		y;
	};

	struct st_Sector_Around
	{
		int count;
		st_Coordinate around[9];
	};

	struct st_Player
	{
		ULONGLONG	lastRecvTime = 0;
		INT64		accountNo = 0;
		SESSION_ID	sessionNo = 0;
		WCHAR		ID[20];
		WCHAR		nickName[20];
		short		curSectorX = -1;
		short		curSectorY = -1;
		bool		bLogin = false;
	};

	struct st_MSG {
		BYTE			type;
		SESSION_ID		sessionNo;
		CNetPacket*		packet;
	};
}