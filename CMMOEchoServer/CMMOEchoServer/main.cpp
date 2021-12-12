#include "CMMOEchoServer.h"
#include "CProfiler.h"
#include "CCrashDump.h"

int main()
{
	procademy::CCrashDump::SetHandlerDump();
	CProfiler::InitProfiler(20);

	procademy::CMMOEchoServer server;

	server.BeginServer();

	CProfiler::DestroyProfiler();

	return 0;
}