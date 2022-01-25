#include "CChatServerSingle.h"
#include <stdio.h>
#include "CCrashDump.h"
#include "CProfiler.h"
#include "memoryDebug.h"

//#define dfTHREAD_NUM (3)
#define dfSERVER_PORT (6000)

procademy::CChatServerSingle server;

std::unordered_map<procademy::CNetPacket*, int> g_mapPacket;
SRWLOCK g_mapLock;

struct memDebug;

unsigned short g_memoryIdx;
memDebug g_memoryDebug[USHRT_MAX + 1];

bool g_btn = false;

int main()
{
	InitializeSRWLock(&g_mapLock);

	procademy::CCrashDump::SetHandlerDump();

	server.BeginServer();

	server.RunServer();

	return 0;
}