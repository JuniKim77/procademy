#include "Sector.h"
#include "User.h"

std::list<User*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void Sector_AddUser(User* user)
{
	int sectorX = user->x / dfSECTOR_SIZE;
	int sectorY = user->y / dfSECTOR_SIZE;

	g_Sector[sectorY][sectorX].push_back(user);
}

void Sector_RemoveUser(User* user)
{
	int sectorX = user->x / dfSECTOR_SIZE;
	int sectorY = user->y / dfSECTOR_SIZE;

	for (auto iter = g_Sector[sectorY][sectorX].begin(); iter != g_Sector[sectorY][sectorX].end(); ++iter)
	{
		if (user == *iter)
		{
			g_Sector[sectorY][sectorX].erase(iter);

			return;
		}
	}
}

bool Sector_UpdateUser(User* user)
{


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


}
