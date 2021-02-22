#pragma once
#include "myNew.h"
#include "logging.h"

myNew::~myNew()
{
	if (mAddr != nullptr)
	{
		writeLog(LOG_TYPE_LEAK, mAddr);
	}
}
