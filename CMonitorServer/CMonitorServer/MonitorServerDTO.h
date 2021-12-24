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

	struct st_ServerClient
	{
		SESSION_ID						sessionNo = 0;
		BYTE							serverNo;
		CSafeQueue<st_MonitorData*>		dataSet[DATA_SET_SZIE];
		int								max[DATA_SET_SZIE];
		int								min[DATA_SET_SZIE];
	};

	struct st_DBData
	{
		int		serverNo;
		int		logtime;
		int		type;
		int		avg;
		int		min;
		int		max;
	};
}