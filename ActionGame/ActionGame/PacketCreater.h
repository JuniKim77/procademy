#pragma once
#include "PacketDefine.h"

void CreateMoveStartPacket(stHeader* header, csMoveStart* packet, BYTE direction, WORD x, WORD y);
void CreateMoveStopPacket(stHeader* header, csMoveStop* packet, BYTE direction, WORD x, WORD y);
void CreateAttack1Packet(stHeader* header, csAttack1* packet, BYTE direction, WORD x, WORD y);
void CreateAttack2Packet(stHeader* header, csAttack2* packet, BYTE direction, WORD x, WORD y);
void CreateAttack3Packet(stHeader* header, csAttack3* packet, BYTE direction, WORD x, WORD y);