#pragma once

#include <wtypes.h>

#define dfNETWORK_PORT 20000
#define dfLOG_LEVEL_DEBUG (0)
#define dfLOG_LEVEL_ERROR (1)
#define dfLOG_LEVEL_NOTICE (2)

class Session;
class CPacket;
struct st_Sector;
struct User;

void CreateServer();
void NetWorkProc();
void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
void DisconnectProc(DWORD sessionKey);
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock, int logLevel = dfLOG_LEVEL_DEBUG);
/// <summary>
/// 특정 섹터 1개에 있는 모든 클라이언트들에게 메시지 보내기
/// </summary>
/// <param name="sectorX"></param>
/// <param name="sectorY"></param>
/// <param name="packet"></param>
/// <param name="exceptSessionNo"></param>
void SendPacket_SectorOne(int sectorX, int sectorY, CPacket* packet, DWORD exceptSessionNo);
/// <summary>
/// 특정 1명 클라에게 메시지 보내기
/// </summary>
/// <param name="to"></param>
/// <param name="packet"></param>
void SendPacket_Unicast(DWORD to, CPacket* packet);
/// <summary>
/// 특정 클라 1명의 주변 섹터에 전부 메시지 보내기
/// </summary>
/// <param name="to"></param>
/// <param name="packet"></param>
/// <param name="sendMyself"></param>
void SendPacket_Around(DWORD to, CPacket* packet, bool sendMyself = false);
/// <summary>
/// 브로드 캐스트 메시지
/// </summary>
/// <param name="packet"></param>
void SendPacket_Broadcast(CPacket* packet);
/// <summary>
/// 섹터 변경시 클라에게 보내야 할 메세지 처리
/// </summary>
/// <param name="user"></param>
void UserSectorUpdatePacket(User* user);