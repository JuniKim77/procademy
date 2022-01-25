#include "CMMOEchoServer.h"
#include "CProfiler.h"
#include "CCrashDump.h"

int main()
{
	procademy::CCrashDump::SetHandlerDump();

	procademy::CMMOEchoServer server;

	server.BeginServer();
	server.RunServer();

	return 0;
}