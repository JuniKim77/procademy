#pragma once

#include <wtypes.h>

#define dfNETWORK_PORT 20000

class Session;
class CPacket;

void CreateServer();
void NetWorkProc();
void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
void DisconnectProc(DWORD sessionKey);
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock);
