#include "CAStar.h"
#include "CSVReader.h"
#include <wtypes.h>

#define MAP_WIDTH (60)
#define MAP_HEIGHT (40)

procademy::CAStar::TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
procademy::CAStar star;

void LoadMap(const WCHAR* filename);

int main()
{
	LoadMap(L"60_40_map.csv");

	g_Map[0][0] = procademy::CAStar::TileType::TILE_TYPE_BEGIN;
	g_Map[39][59] = procademy::CAStar::TileType::TILE_TYPE_END;

	

	procademy::CAStar::Coordi begin = { 0,0 };
	procademy::CAStar::Coordi end = { 59, 39 };

	procademy::CAStar::Node* ret = star.SearchDestination(begin, end);



	return 0;
}

void LoadMap(const WCHAR* filename)
{
	CSVFile csv(filename);
	WCHAR pBuf;

	csv.SelectRow(2);

	for (int i = 0; i < MAP_HEIGHT; ++i)
	{
		for (int j = 0; j < MAP_WIDTH; ++j)
		{
			int ret = csv.GetColumn(&pBuf, 1);

			if (pBuf == L'*')
			{
				g_Map[i][j] = procademy::CAStar::TileType::TILE_TYPE_WALL;
				pBuf = L'@';
			}
			else
			{
				g_Map[i][j] = procademy::CAStar::TileType::TILE_TYPE_PATH;
			}

			csv.NextColumn();
		}
	}
}
