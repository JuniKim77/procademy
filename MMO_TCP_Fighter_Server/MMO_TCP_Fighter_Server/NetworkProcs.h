#pragma once

#include <wtypes.h>

#define dfNETWORK_PORT 20000
#define dfLOG_LEVEL_DEBUG (0)
#define dfLOG_LEVEL_ERROR (1)
#define dfLOG_LEVEL_NOTICE (2)

class Session;
class CPacket;

void CreateServer();
void NetWorkProc();
void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
void DisconnectProc(DWORD sessionKey);
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock, int logLevel = dfLOG_LEVEL_DEBUG);
