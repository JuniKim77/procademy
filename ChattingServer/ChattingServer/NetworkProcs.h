#pragma once

#include <wtypes.h>
#include <unordered_map>

class Session;
class CPacket;

void CreateServer();
void NetWorkProc();
void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
void AcceptProc();