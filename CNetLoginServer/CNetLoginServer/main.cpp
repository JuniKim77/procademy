#include "CNetLoginServer.h"
#include "CCrashDump.h"
#include "CProfiler.h"

struct sessionDebug
{
	UINT64 playerNo;
	UINT64 sessionNo;
	UINT64 lastTime;
	UINT64 threadId;
	int type;
	int loginID;
};

sessionDebug g_sessionLog[USHRT_MAX + 1];
USHORT g_sessionIdx;

void _sessionLog(
	UINT64 playerNo,
	UINT64 sessionNo,
	DWORD lastTime,
	DWORD threadId,
	int type,
	int loginID
)
{
	USHORT index = InterlockedIncrement16((short*)&g_sessionIdx);

	g_sessionLog[index].playerNo = playerNo;
	g_sessionLog[index].sessionNo = sessionNo;
	g_sessionLog[index].lastTime = lastTime;
	g_sessionLog[index].threadId = threadId;
	g_sessionLog[index].type = type;
	g_sessionLog[index].loginID = loginID;
}

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	procademy::CNetLoginServer server;

	server.BeginServer();

	server.RunServer();

	return 0;
}