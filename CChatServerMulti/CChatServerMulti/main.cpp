#include "CChatServerMulti.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

int main()
{
	CProfiler::InitProfiler(30);

	procademy::CCrashDump::SetHandlerDump();

	procademy::CChatServerMulti server;

	server.BeginServer();

	CProfiler::DestroyProfiler();

	return 0;
}