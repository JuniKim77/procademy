#pragma once

#include <wtypes.h>

class CPacket;
struct st_PACKET_HEADER;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet);

bool ReqLogin(DWORD from, CPacket* packet);
void ResLogin(DWORD to, BYTE result);
bool ReqRoomList(DWORD from, CPacket* packet);
void ResRoomList(DWORD to, DWORD roomNo);
bool ReqRoomCreate(DWORD from, CPacket* packet);
void ResRoomCreate(DWORD to, BYTE result, DWORD roomNo);
bool ReqRoomEnter(DWORD from, CPacket* packet);
bool ReqChat(DWORD to, CPacket* packet);
bool ReqRoomLeave(DWORD from, CPacket* packet);

void SendUnicast(DWORD to, st_PACKET_HEADER* header, CPacket* packet);
void SendBroadcast(st_PACKET_HEADER* header, CPacket* packet);
void SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet);
