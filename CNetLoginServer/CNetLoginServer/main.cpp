#include "CNetLoginServer.h"
#include "CCrashDump.h"
#include "CProfiler.h"

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	procademy::CNetLoginServer server;

	server.BeginServer();

	server.RunServer();

	return 0;
}