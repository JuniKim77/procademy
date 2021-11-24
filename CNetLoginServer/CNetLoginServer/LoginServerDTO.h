#pragma once

namespace procademy
{
	enum {
		NAME_MAX = 20
	};

	struct st_User
	{
		ULONGLONG	lastRecvTime = 0;
		INT64		accountNo = 0;
		SESSION_ID	sessionNo = 0;
		char		SessionKey[64];
		WCHAR		ID[NAME_MAX];
		WCHAR		nickName[NAME_MAX];
	};
}