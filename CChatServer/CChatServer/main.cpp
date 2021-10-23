#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

int main()
{
	CProfiler::InitProfiler(10);

	procademy::CCrashDump::SetHandlerDump();

	/*procademy::CEchoServerNoLock server;

	server.Start(dfSERVER_PORT, dfTHREAD_NUM, dfTHREAD_NUM, false, 200);

	server.WaitForThreadsFin();*/

	//CDebugger::PrintLogOut(L"Debug.txt");

	return 0;
}