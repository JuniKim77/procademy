#include "CChatServerSingle.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

int main()
{
	CProfiler::InitProfiler(10);

	procademy::CCrashDump::SetHandlerDump();

	procademy::CChatServerSingle server;

	server.BeginServer();

	return 0;
}