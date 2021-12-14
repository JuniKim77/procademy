#pragma once

namespace procademy
{
	struct st_MonitorClient
	{

	};

	struct st_ServerClient
	{
		SESSION_ID	sessionNo = 0;
		bool		bLogin = false;
		int			serverNo;
	};
}