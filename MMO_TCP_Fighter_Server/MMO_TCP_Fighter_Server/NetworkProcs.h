#pragma once

#include <wtypes.h>
#include "CLogger.h"
//#define DEBUG

class Session;
class CPacket;
struct st_Sector;
struct User;

void CreateServer();
void NetWorkProc();
void SelectProc(DWORD* keyTable, FD_SET* rset, FD_SET* wset);
void DisconnectProc(DWORD sessionKey);
void AcceptProc();
void LogError(const WCHAR* msg, SOCKET sock, int logLevel = dfLOG_LEVEL_ERROR);
/// <summary>
/// Ư�� ���� 1���� �ִ� ��� Ŭ���̾�Ʈ�鿡�� �޽��� ������
/// </summary>
/// <param name="sectorX"></param>
/// <param name="sectorY"></param>
/// <param name="packet"></param>
/// <param name="exceptSessionNo"></param>
void SendPacket_SectorOne(int sectorX, int sectorY, CPacket* packet, DWORD exceptSessionNo);
/// <summary>
/// Ư�� 1�� Ŭ�󿡰� �޽��� ������
/// </summary>
/// <param name="to"></param>
/// <param name="packet"></param>
void SendPacket_Unicast(DWORD to, CPacket* packet);
/// <summary>
/// Ư�� Ŭ�� 1���� �ֺ� ���Ϳ� ���� �޽��� ������
/// </summary>
/// <param name="to"></param>
/// <param name="packet"></param>
/// <param name="sendMyself"></param>
void SendPacket_Around(DWORD to, CPacket* packet, bool sendMyself = false);
/// <summary>
/// ��ε� ĳ��Ʈ �޽���
/// </summary>
/// <param name="packet"></param>
void SendPacket_Broadcast(CPacket* packet);
/// <summary>
/// ���� ����� Ŭ�󿡰� ������ �� �޼��� ó��
/// </summary>
/// <param name="user"></param>
void UserSectorUpdatePacket(User* user);