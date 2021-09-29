#pragma once

#define dfLOG_LEVEL_DEBUG (0)
#define dfLOG_LEVEL_ERROR (1)
#define dfLOG_LEVEL_NOTICE (2)
#define dfLOG_LEVEL_OFF (3)

#include <wtypes.h>

class CLogger
{
public:
	CLogger() = delete;
	static void _Log(int logLevel, const WCHAR* format, ...);
	static void Initialize();
	static void SetDirectory(const WCHAR* path);
	static void setLogLevel(int level) { mLogLevel = level; }
	static int getLogLevel() { return mLogLevel; }

private:
	static void LockFile();
	static void UnlockFile();

private:
	static int mLogLevel;
	static SRWLOCK mSrwLock;
	static WCHAR mFolderPath[MAX_PATH];
	static DWORD mLogCount;
};