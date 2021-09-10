#pragma once

#define dfLOG_LEVEL_DEBUG (0)
#define dfLOG_LEVEL_ERROR (1)
#define dfLOG_LEVEL_NOTICE (2)

#include <wtypes.h>

class CLogger
{
public:
	CLogger() = delete;
	static void _Log(int logLevel, const WCHAR* format, ...);
	static void setLogLevel(int level) { mLogLevel = level; }
	static int getLogLevel() { return mLogLevel; }

private:
	static int mLogLevel;
};