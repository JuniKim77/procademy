#pragma once
#include "PacketDefine.h"

class CPacket;

void CreateMoveStartPacket(CPacket* packet, BYTE direction, WORD x, WORD y);
void CreateMoveStopPacket(CPacket* packet, BYTE direction, WORD x, WORD y);
void CreateAttack1Packet(CPacket* packet, BYTE direction, WORD x, WORD y);
void CreateAttack2Packet(CPacket* packet, BYTE direction, WORD x, WORD y);
void CreateAttack3Packet(CPacket* packet, BYTE direction, WORD x, WORD y);
void LogPacket(CPacket* packet, DWORD id);