#include <stdio.h>
#include "CEchoServerNoLock.h"
#include "CCrashDump.h"
#include "CProfiler.h"

#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

int main()
{
	CProfiler::InitProfiler(30);

	procademy::CCrashDump::SetHandlerDump();

	procademy::CEchoServerNoLock server;

	server.BeginServer();

	//CDebugger::PrintLogOut(L"Debug.txt");

	return 0;
}