#include "CChatServerSingle.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"

//#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

procademy::CChatServerSingle server;

std::unordered_map<procademy::CNetPacket*, int> g_mapPacket;
SRWLOCK g_mapLock;

int main()
{
	InitializeSRWLock(&g_mapLock);
	//CProfiler::InitProfiler(30);

	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();

	CProfiler::DestroyProfiler();

	return 0;
}