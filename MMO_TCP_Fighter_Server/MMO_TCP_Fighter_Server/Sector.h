#pragma once

//#define DEBUG
#include <list>
#include "ActionDefine.h"

struct st_Sector
{
	int x;
	int y;
};

struct st_Sector_Around
{
	int count;
	st_Sector around[9];
};

struct User;

extern std::list<User*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

/// <summary>
/// ������ ���� ��ǥ�� ���� ��ǥ�� ��ȯ�� �߰��ϴ� �Լ�
/// </summary>
/// <param name="user"></param>
void Sector_AddUser(User* user);
/// <summary>
/// ������ ���� ��ǥ�� ���� ��ǥ�� ��ȯ�� �����ϴ� �Լ�
/// </summary>
/// <param name="user"></param>
void Sector_RemoveUser(User* user, bool isCurrent = false);
/// <summary>
/// ������ ���� �̵��� ���� ���� ������Ʈ
/// </summary>
/// <param name="user"></param>
/// <returns></returns>
bool Sector_UpdateUser(User* user);
/// <summary>
/// ���� ��ǥ ���� 8���� ���� �������� �Լ�
/// </summary>
/// <param name="x">���� ��ǥ x</param>
/// <param name="y">���� ��ǥ y</param>
/// <param name="output"></param>
void GetSectorAround(int x, int y, st_Sector_Around* output);
/// <summary>
/// ���� ���� �ֺ� ��ǥ ��ȭ�� �������� �Լ�
/// </summary>
/// <param name="user">����</param>
/// <param name="pRemoveSector">���� �� ���͵�</param>
/// <param name="pAddSector">�߰� �� ���͵�</param>
void GetUpdateSectorAround(User* user, st_Sector_Around* pRemoveSector, st_Sector_Around* pAddSector);