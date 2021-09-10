#pragma once

#include <wtypes.h>

class CPacket;
struct stHeader;

void cpSC_CreateMyUser(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y, BYTE hp);
void cpSC_CreateOtherUser(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y, BYTE hp);
void cpSC_DeleteUser(CPacket* packet, DWORD id);
void cpSC_MoveStart(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y);
void cpSC_MoveStop(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y);
void cpSC_Attack1(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y);
void cpSC_Attack2(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y);
void cpSC_Attack3(CPacket* packet, DWORD id, BYTE direction, WORD x, WORD y);
void cpSC_Damage(CPacket* packet, DWORD attackID, DWORD damageID, BYTE damageHP);
void cpSC_Synchronize(CPacket* packet, DWORD id, WORD x, WORD y);
void cpSC_Echo(CPacket* packet, DWORD time);
void FillHeader(stHeader* header, BYTE msgType, BYTE msgSize);
