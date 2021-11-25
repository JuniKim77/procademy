#pragma once

namespace procademy
{
	enum {
		en_NAME_MAX = 20
	};

	struct st_Player
	{
		ULONGLONG	lastRecvTime = 0;
		INT64		accountNo = 0;
		SESSION_ID	sessionNo = 0;
		char		SessionKey[64];
		WCHAR		ID[en_NAME_MAX];
		WCHAR		nickName[en_NAME_MAX];
	};
}