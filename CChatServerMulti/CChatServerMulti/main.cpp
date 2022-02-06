#include "CChatServerMulti.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

procademy::CChatServerMulti server;

int main()
{
	//CProfiler::InitProfiler(30);

	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();
	server.RunServer();

	return 0;
}