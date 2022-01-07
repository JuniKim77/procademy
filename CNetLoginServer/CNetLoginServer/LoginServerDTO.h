#pragma once

namespace procademy
{
	enum {
		en_NAME_MAX = 20
	};

	struct st_Player
	{
		DWORD		lastRecvTime = 0;
		INT64		accountNo = 0;
		SESSION_ID	sessionNo = 0;
		char		SessionKey[64];
		WCHAR		ID[en_NAME_MAX];
		WCHAR		nickName[en_NAME_MAX];
		BYTE		dummyIndex = 0;
	};

	struct st_DummyServerInfo
	{
		WCHAR		IP[16];
		WCHAR		GameServerIP[16];
		USHORT		GameServerPort;
		WCHAR		ChatServerIP[16];
		USHORT		ChatServerPort;
	};
}