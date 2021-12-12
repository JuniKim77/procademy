#include "CChatServerSingle.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

//#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

alignas(64) procademy::CChatServerSingle server;

int main()
{
	//CProfiler::InitProfiler(30);

	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();

	CProfiler::DestroyProfiler();

	return 0;
}