#pragma once
#include "logging.h"
#include "myNew.h"

myNew::~myNew()
{
	if (mAddr != nullptr)
	{
		writeLog(LOG_TYPE_LEAK, this);
	}
}
