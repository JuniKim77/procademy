#include "CNetLoginServer.h"
#include "CCrashDump.h"
#include "CProfiler.h"

int main()
{
	CProfiler::InitProfiler(80);

	procademy::CCrashDump::SetHandlerDump();

	procademy::CNetLoginServer server;

	server.BeginServer();

	CProfiler::DestroyProfiler();

	return 0;
}