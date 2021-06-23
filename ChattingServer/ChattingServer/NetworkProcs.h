#pragma once

#include <wtypes.h>
#include <unordered_map>

class Session;

void CreateServer();
void NetWorkProc();
void SelectProc(std::unordered_map<DWORD, Session*>::iterator iter, FD_SET* rset);
void DestroySessionProc();
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock);