#pragma once

#include <wtypes.h>

class CPacket;
struct stHeader;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet);