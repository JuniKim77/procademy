#include "Sector.h"
#include "User.h"
#include "CLogger.h"

std::list<User*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];
extern CLogger g_Logger;

void Sector_AddUser(User* user)
{
	int sectorX = user->curSector.x;
	int sectorY = user->curSector.y;

	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[UserNo: %d] Add to Sector [X: %d][Y: %d]\n",
		user->userNo, sectorX, sectorY);
	g_Sector[sectorY][sectorX].push_back(user);
}

void Sector_RemoveUser(User* user, bool isCurrent)
{
	int sectorX;
	int sectorY;

	if (isCurrent)
	{
		sectorX = user->curSector.x;
		sectorY = user->curSector.y;
	}
	else
	{
		sectorX = user->oldSector.x;
		sectorY = user->oldSector.y;
	}
	

	for (auto iter = g_Sector[sectorY][sectorX].begin(); iter != g_Sector[sectorY][sectorX].end(); ++iter)
	{
		if (user == *iter)
		{
			g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[UserNo: %d] Remove from Sector [X: %d][Y: %d]\n",
				user->userNo, sectorX, sectorY);
			g_Sector[sectorY][sectorX].erase(iter);

			return;
		}
	}

	g_Logger._Log(dfLOG_LEVEL_NOTICE, L"[UserNo: %d] Remove Not Found [X: %d][Y: %d]\n",
		user->userNo, sectorX, sectorY);
}

bool Sector_UpdateUser(User* user)
{
	int sectorX = user->x / dfSECTOR_SIZE;
	int sectorY = user->y / dfSECTOR_SIZE;

	if (sectorX != user->curSector.x || sectorY != user->curSector.y)
	{
		user->oldSector = user->curSector;
		user->curSector.x = sectorX;
		user->curSector.y = sectorY;

		Sector_AddUser(user);
		Sector_RemoveUser(user);

		//st_Sector_Around sectorAround;
		//GetSectorAround(sectorX, sectorY, &sectorAround);

		//// 주변 섹터 상태 출력
		//for (int i = 0; i < sectorAround.count; ++i)
		//{
		//	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[%d] [X: %d][Y: %d]: ", i, sectorAround.around[i].x, sectorAround.around[i].y);
		//	
		//	for (auto iter = g_Sector[sectorAround.around[i].y][sectorAround.around[i].x].begin();
		//		iter != g_Sector[sectorAround.around[i].y][sectorAround.around[i].x].end(); ++iter)
		//	{
		//		g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[User: %d] ", (*iter)->userNo);
		//	}
		//	g_Logger._Log(dfLOG_LEVEL_DEBUG, L"\n");
		//}
		//g_Logger._Log(dfLOG_LEVEL_DEBUG, L"=======================\n");

		return true;
	}
	
	return false;
}

void GetSectorAround(int x, int y, st_Sector_Around* output)
{
	--x;
	--y;

	output->count = 0;

	for (int i = 0; i < 3; ++i)
	{
		if (y + i < 0 || y + i >= dfSECTOR_MAX_Y)
			continue;

		for (int j = 0; j < 3; ++j)
		{
			if (x + j < 0 || x + j >= dfSECTOR_MAX_X)
				continue;

			output->around[output->count].x = x + j;
			output->around[output->count].y = y + i;
			output->count++;
		}
	}
}

void GetUpdateSectorAround(User* user, st_Sector_Around* pRemoveSector, st_Sector_Around* pAddSector)
{
	st_Sector_Around oldSectorAround;
	st_Sector_Around curSectorAround;

	oldSectorAround.count = 0;
	curSectorAround.count = 0;

	pRemoveSector->count = 0;
	pAddSector->count = 0;

	GetSectorAround(user->oldSector.x, user->oldSector.y, &oldSectorAround);
	GetSectorAround(user->curSector.x, user->curSector.y, &curSectorAround);

	for (int cntOld = 0; cntOld < oldSectorAround.count; ++cntOld)
	{
		bool bFound = false;
		for (int cntCur = 0; cntCur < curSectorAround.count; ++cntCur)
		{
			if (oldSectorAround.around[cntOld].x == curSectorAround.around[cntCur].x &&
				oldSectorAround.around[cntOld].y == curSectorAround.around[cntCur].y)
			{
				bFound = true;
				break;
			}
		}

		if (bFound == false)
		{
			pRemoveSector->around[pRemoveSector->count] = oldSectorAround.around[cntOld];
			pRemoveSector->count++;
		}
	}

	for (int cntCur = 0; cntCur < curSectorAround.count; ++cntCur)
	{
		bool bFound = false;
		for (int cntOld = 0; cntOld < oldSectorAround.count; ++cntOld)
		{
			if (oldSectorAround.around[cntOld].x == curSectorAround.around[cntCur].x &&
				oldSectorAround.around[cntOld].y == curSectorAround.around[cntCur].y)
			{
				bFound = true;
				break;
			}
		}

		if (bFound == false)
		{
			pAddSector->around[pAddSector->count] = curSectorAround.around[cntCur];
			pAddSector->count++;
		}
	}
}

