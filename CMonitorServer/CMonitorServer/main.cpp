#include "CMonitorServer.h"
#include "CCrashDump.h"
#include "memoryDebug.h"

procademy::CMonitorServer server;

struct memDebug;

unsigned short g_memoryIdx;
memDebug g_memoryDebug[USHRT_MAX + 1];

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();

	return 0;
}