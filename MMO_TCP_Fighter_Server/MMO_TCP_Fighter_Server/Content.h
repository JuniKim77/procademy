#pragma once

#include <wtypes.h>

class CPacket;
struct stHeader;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet);
bool CS_MoveStart(DWORD from, CPacket* packet);
bool CS_MoveStop(DWORD from, CPacket* packet);
bool CS_Attack1(DWORD from, CPacket* packet);
bool CS_Attack2(DWORD from, CPacket* packet);
bool CS_Attack3(DWORD from, CPacket* packet);
bool CS_Echo(DWORD from, CPacket* packet);
bool UserMoveCheck(int x, int y);