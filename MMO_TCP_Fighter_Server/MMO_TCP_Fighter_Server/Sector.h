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
/// 유저의 현재 좌표를 섹터 좌표로 변환해 추가하는 함수
/// </summary>
/// <param name="user"></param>
void Sector_AddUser(User* user);
/// <summary>
/// 유저의 현재 좌표를 섹터 좌표로 변환해 삭제하는 함수
/// </summary>
/// <param name="user"></param>
void Sector_RemoveUser(User* user, bool isCurrent = false);
/// <summary>
/// 유저의 섹터 이동시 섹터 정보 업데이트
/// </summary>
/// <param name="user"></param>
/// <returns></returns>
bool Sector_UpdateUser(User* user);
/// <summary>
/// 섹터 좌표 기준 8방향 섹터 가져오는 함수
/// </summary>
/// <param name="x">섹터 좌표 x</param>
/// <param name="y">섹터 좌표 y</param>
/// <param name="output"></param>
void GetSectorAround(int x, int y, st_Sector_Around* output);
/// <summary>
/// 유저 기준 주변 좌표 변화를 가져오는 함수
/// </summary>
/// <param name="user">유저</param>
/// <param name="pRemoveSector">삭제 된 섹터들</param>
/// <param name="pAddSector">추가 된 섹터들</param>
void GetUpdateSectorAround(User* user, st_Sector_Around* pRemoveSector, st_Sector_Around* pAddSector);