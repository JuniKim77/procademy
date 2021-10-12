#include <stdio.h>
#include "CEchoServerNoLock.h"
#include "CDebugger.h"
#include "CCrashDump.h"

#define dfSERVER_PORT (6000)

int main()
{
	CDebugger::SetDirectory(L"./debugs");
	CDebugger::Initialize();
	procademy::CCrashDump::CCrashDump();

	CEchoServerNoLock server;

	server.Start(dfSERVER_PORT, 4, 4, false, 200);

	server.WaitForThreadsFin();

	//CDebugger::PrintLogOut(L"Debug.txt");

	return 0;
}