#pragma once
#pragma once
#pragma warning(disable:26495)

namespace procademy
{
	struct st_Player;

	struct st_Sector
	{
		std::list<st_Player*>	list;
		int						lockIndex;
		SRWLOCK					sectorLock;
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

	enum {
		en_NAME_MAX = 20
	};

	struct st_Player
	{
		ULONGLONG	lastRecvTime = 0;
		INT64		accountNo = 0;
		SESSION_ID	sessionNo = 0;
		WCHAR		ID[en_NAME_MAX];
		WCHAR		nickName[en_NAME_MAX];
		short		curSectorX = -1;
		short		curSectorY = -1;
		bool		bLogin = false;
	};

	struct st_MSG {
		BYTE			type;
		SESSION_ID		sessionNo;
		CNetPacket* packet;
		int				mCounter = 0;

		st_MSG() {
			InterlockedIncrement((long*)&mCounter);

			if (mCounter > 1)
				CRASH();
		}
	};
}