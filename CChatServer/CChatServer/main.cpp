#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"
#include "CChatServerSingle.h"

#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

int main()
{
	CProfiler::InitProfiler(10);

	procademy::CCrashDump::SetHandlerDump();

	procademy::CChatServerSingle server;

	server.Start(dfSERVER_PORT, dfTHREAD_NUM, dfTHREAD_NUM, false, 200);

	server.WaitForThreadsFin();

	return 0;
}