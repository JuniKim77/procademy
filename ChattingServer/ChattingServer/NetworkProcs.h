#pragma once

#include <wtypes.h>

void CreateServer();
void NetWorkProc();
void AcceptProc();
void SelectProc();
void LogError(const WCHAR* msg, SOCKET sock);