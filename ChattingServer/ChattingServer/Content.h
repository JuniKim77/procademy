#pragma once

#include <wtypes.h>

class CPacket;
struct st_PACKET_HEADER;

bool PacketProc(DWORD from, WORD msgType, CPacket* packet);

bool ReqLogin(DWORD from, CPacket* packet);
void ResLogin(DWORD to, BYTE result);
bool ReqRoomList(DWORD from, CPacket* packet);
void ResRoomList(DWORD to);
bool ReqRoomCreate(DWORD from, CPacket* packet);
void ResRoomCreate(DWORD to, BYTE result, DWORD roomNo);
bool ReqRoomEnter(DWORD from, CPacket* packet);
void ResRoomEnter(DWORD from, BYTE result, DWORD roomNo);
void ResRoomOtherUserEnter(DWORD client, DWORD roomNo);
bool ReqChat(DWORD client, CPacket* packet);
void ResChat(DWORD from, DWORD roomNo, WORD msgSize, const WCHAR* msg);
bool ReqRoomLeave(DWORD from);
void ResRoomLeave(DWORD client, DWORD roomNo);
void ResRoomDelete(DWORD roomNo);
bool ReqStressEcho(DWORD from, CPacket* packet);

void SendUnicast(DWORD to, st_PACKET_HEADER* header, CPacket* packet);
void SendBroadcast(st_PACKET_HEADER* header, CPacket* packet);
void SendBroadcast_room(DWORD roomNo, DWORD from, st_PACKET_HEADER* header, CPacket* packet);
