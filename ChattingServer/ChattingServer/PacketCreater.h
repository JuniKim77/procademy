#pragma once
#include <wtypes.h>

class CPacket;
struct st_PACKET_HEADER;

void CreateResLoginPacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD userNo);
void CreateResRoomListPacket(st_PACKET_HEADER* header, CPacket* packet);
void CreateResRoomCreatePacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD roomNo);
void CreateResRoomEnterPacket(st_PACKET_HEADER* header, CPacket* packet, BYTE result, DWORD roomNo);
void CreateResChatPacket(st_PACKET_HEADER* header, CPacket* packet, DWORD from, WORD msgSize, const WCHAR* msg);
void CreateResRoomLeavePacket(st_PACKET_HEADER* header, CPacket* packet, DWORD from);
void CreateResRoomDeletePacket(st_PACKET_HEADER* header, CPacket* packet, DWORD roomNo);
void CreateResOtherUserRoomEnterPacket(st_PACKET_HEADER* header, CPacket* packet, const WCHAR* name, DWORD userNo);
void CreateResStressEcho(st_PACKET_HEADER* header, CPacket* packet, CPacket* receivePacket);
BYTE makeCheckSum(CPacket* packet, WORD msgType);
void FillHeader(st_PACKET_HEADER* header, CPacket* packet, WORD msgType);