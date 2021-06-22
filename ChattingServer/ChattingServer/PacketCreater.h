#pragma once
#include "Protocol.h"

class CPacket;

void CreateSCLoginPacket(CPacket* packet, BYTE result, DWORD userNo);
void CreateSCRoomListPacket(CPacket* packet, BYTE result, DWORD userNo);
