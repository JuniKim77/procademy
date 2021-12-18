#include "CMonitorServer.h"
#include "CCrashDump.h"

procademy::CMonitorServer server;

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();

	return 0;
}