#pragma once
#include "Protocol.h"

class CPacket;

void CreateResLoginPacket(CPacket* packet, BYTE result, DWORD userNo);
void CreateResRoomListPacket(CPacket* packet, DWORD roomNo);
void CreateResRoomCreatePacket(CPacket* packet, BYTE result, DWORD roomNo);
void CreateResRoomEnterPacket(CPacket* packet, BYTE result, DWORD roomNo);
void CreateResChatPacket(CPacket* packet, DWORD from, WORD msgSize, const WCHAR* msg);
void CreateResRoomLeavePacket(CPacket* packet, DWORD from);
void CreateResRoomDeletePacket(CPacket* packet, DWORD roomNo);
void CreateResOtherUserRoomEnterPacket(CPacket* packet, const WCHAR* name, DWORD userNo);
