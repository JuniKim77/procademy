#pragma once

#define dfLOG_LEVEL_DEBUG (0)
#define dfLOG_LEVEL_ERROR (1)
#define dfLOG_LEVEL_NOTICE (2)

#include <wtypes.h>

class CLogger
{
public:
	CLogger() {}
	void _Log(int logLevel, const WCHAR* format, ...);
	void setLogLevel(int level) { mLogLevel = level; }

private:
	int mLogLevel = dfLOG_LEVEL_DEBUG;
};