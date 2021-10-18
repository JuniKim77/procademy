#include <stdio.h>
#include "CEchoServerNoLock.h"
#include "CCrashDump.h"

#define dfSERVER_PORT (6000)

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	CEchoServerNoLock server;

	server.Start(dfSERVER_PORT, 2, 2, false, 200);

	server.WaitForThreadsFin();

	//CDebugger::PrintLogOut(L"Debug.txt");

	return 0;
}