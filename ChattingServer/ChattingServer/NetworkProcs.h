#pragma once

#include <wtypes.h>

void CreateServer();
void NetWorkProc();
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock);