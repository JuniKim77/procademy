#pragma once
#include "CSafeQueue.h"

#define DATA_SET_SZIE (45)

namespace procademy
{
	struct st_MonitorData
	{
		BYTE	type;
		int		value;
		int		timeStamp;
	};

	struct st_MonitorClient
	{
		SESSION_ID						sessionNo = 0;
	};

	struct st_ServerClient
	{
		SESSION_ID						sessionNo = 0;
		BYTE							serverNo;
		CSafeQueue<st_MonitorData*>		dataSet[DATA_SET_SZIE];
	};
}