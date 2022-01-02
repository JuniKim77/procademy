#include "FilePathDTO.h"
#include "CAStar.h"
#include "CSVReader.h"
#include <wtypes.h>
#include "CJumpPointSearch.h"
#include <time.h>
#include "CProfiler.h"
#include <stack>
#include <process.h>

using namespace std;

#define dfTHREAD_NUM (4)
#define dfCHUNK_SIZE (10000)

procademy::TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
procademy::CAStar star;
procademy::CJumpPointSearch jump;

unsigned int WINAPI workerThread(LPVOID arg);

void LoadMap(const WCHAR* filename);
void GenerateRand(procademy::Coordi& begin, procademy::Coordi& end);

int main()
{
	srand(time(NULL));
	CProfiler::InitProfiler(10);
	stack<pair<procademy::Coordi, procademy::Coordi>> stk[dfTHREAD_NUM];
	stack<pair<procademy::Coordi, procademy::Coordi>> stk2;
	stack<pair<procademy::Coordi, procademy::Coordi>> stk3;
	stack<pair<procademy::Coordi, procademy::Coordi>> stk4;
	HANDLE handles[dfTHREAD_NUM];

	LoadMap(L"60_40_map.csv");

	for (int i = 0; i < dfTHREAD_NUM * dfCHUNK_SIZE; ++i)
	{
		procademy::Coordi begin;
		procademy::Coordi end;

		GenerateRand(begin, end);

		stk2.push({ begin, end });
		stk3.push({ begin, end });
		stk4.push({ begin, end });

		int idx = i % dfTHREAD_NUM;

		stk[idx].push({ begin, end });
	}

	while (stk2.empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk2.top();
		stk2.pop();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret = star.SearchDestination(cur.first, cur.second);

		procademy::Node* ret2 = jump.JumpPointSearch(cur.first, cur.second);

		star.Clear();
		jump.Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}

	int sumAstar = 0;

	CProfiler::Begin(L"SINGLE_ASTAR");
	while (stk3.empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk3.top();
		stk3.pop();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret = star.SearchDestination(cur.first, cur.second);

		sumAstar += star.Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}
	CProfiler::End(L"SINGLE_ASTAR");

	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"AStar Node Create AVG : %d", sumAstar / dfCHUNK_SIZE);

	int sumJump = 0;

	CProfiler::Begin(L"SINGLE_JUMP");
	while (stk4.empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk4.top();
		stk4.pop();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret2 = jump.JumpPointSearch(cur.first, cur.second);

		sumJump += jump.Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}
	CProfiler::End(L"SINGLE_JUMP");

	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Jump Node Create AVG : %d", sumJump / dfCHUNK_SIZE);

	//for (int i = 0; i < dfTHREAD_NUM; ++i)
	//{
	//	handles[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, &stk[i], 0, nullptr);
	//}

	//DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, handles, true, INFINITE);

	CProfiler::Print();

	//switch (retval)
	//{
	//case WAIT_FAILED:
	//	wprintf_s(L"Main Thread Handle Error\n");
	//	break;
	//case WAIT_TIMEOUT:
	//	wprintf_s(L"Main Thread Timeout Error\n");
	//	break;
	//case WAIT_OBJECT_0:
	//	wprintf_s(L"None Error\n");
	//	break;
	//default:
	//	break;
	//}

	return 0;
}

unsigned int __stdcall workerThread(LPVOID arg)
{
	stack<pair<procademy::Coordi, procademy::Coordi>>* stk = (stack<pair<procademy::Coordi, procademy::Coordi>>*)arg;
	stack<pair<procademy::Coordi, procademy::Coordi>> stk2;
	stack<pair<procademy::Coordi, procademy::Coordi>> stk3;
	procademy::CAStar* star = new procademy::CAStar;
	procademy::CJumpPointSearch* jump = new procademy::CJumpPointSearch;

	while (stk->empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk->top();
		stk->pop();
		stk2.push(cur);
		stk3.push(cur);

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret = star->SearchDestination(cur.first, cur.second);

		procademy::Node* ret2 = jump->JumpPointSearch(cur.first, cur.second);

		star->Clear();
		jump->Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}

	CProfiler::Begin(L"MULTI_ASTAR");
	while (stk2.empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk2.top();
		stk2.pop();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret = star->SearchDestination(cur.first, cur.second);

		star->Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}
	CProfiler::End(L"MULTI_ASTAR");

	CProfiler::Begin(L"MULTI_JUMP");
	while (stk3.empty() == false)
	{
		pair<procademy::Coordi, procademy::Coordi> cur = stk3.top();
		stk3.pop();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_BEGIN;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_END;

		procademy::Node* ret2 = jump->JumpPointSearch(cur.first, cur.second);

		jump->Clear();

		g_Map[cur.first.y][cur.first.x] = procademy::TileType::TILE_TYPE_PATH;
		g_Map[cur.second.y][cur.second.x] = procademy::TileType::TILE_TYPE_PATH;
	}
	CProfiler::End(L"MULTI_JUMP");

	delete star;
	delete jump;

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
				g_Map[i][j] = procademy::TileType::TILE_TYPE_WALL;
				pBuf = L'@';
			}
			else
			{
				g_Map[i][j] = procademy::TileType::TILE_TYPE_PATH;
			}

			csv.NextColumn();
		}
	}
}

void GenerateRand(procademy::Coordi& begin, procademy::Coordi& end)
{
	while (1)
	{
		int _X = rand() % 20;
		int _Y = rand() % 40;

		if (g_Map[_Y][_X] == procademy::TileType::TILE_TYPE_PATH)
		{
			begin.x = _X;
			begin.y = _Y;

			break;
		}
	}

	while (1)
	{
		int _X = rand() % 20 + 40;
		int _Y = rand() % 40;

		if (g_Map[_Y][_X] == procademy::TileType::TILE_TYPE_PATH)
		{
			end.x = _X;
			end.y = _Y;

			break;
		}
	}
}
