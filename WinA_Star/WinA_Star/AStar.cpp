#include "AStar.h"
#include "MyHeap.h"
#include "myList.h"
#include <string.h>

#define abs(a) ((a) < 0 ? (-a) : (a))

extern TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
MyHeap g_openList(128);
myList<Node*> g_closeList;
bool closeVisit[MAP_HEIGHT][MAP_WIDTH];
bool openAdded[MAP_HEIGHT][MAP_WIDTH];

bool SearchDestination(Coordi begin, Coordi end)
{
	memset(closeVisit, 0, sizeof(closeVisit));
	memset(openAdded, 0, sizeof(openAdded));
	g_closeList.clear();
	g_openList.ClearHeap();

	const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const float gWeight[8] = { 1.0, 1.5, 1.0, 1.5, 1.0, 1.5, 1.0, 1.5 };

	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node node(begin, 0, h, h);
	g_openList.InsertData(&node);
	openAdded[begin.y][begin.x] = true;

	while (1)
	{
		Node* cur = g_openList.GetMin();
		g_closeList.push_back(cur);
		closeVisit[cur->position.y][cur->position.x] = true;

		if (cur == nullptr)
			break;

		if (cur->position == end)
		{
			// Rendoring
			return true;
		}

		for (int i = 0; i < 8; ++i)
		{
			int nX = cur->position.x + dx[i];
			int nY = cur->position.y + dy[i];

			if (nX < 0 || nY < 0 ||
				nX >= MAP_WIDTH || nY >= MAP_HEIGHT ||
				closeVisit[nY][nX] || 
				g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				continue;
			}

			float nG = cur->g + gWeight[i];
			float nH = abs(nX - end.x) + abs(nY - end.y);
			float nF = nG + nH;

			Node next({ nX, nY }, nG, nH, nF);

			next.pParent = cur;

			if (openAdded[nY][nX])
			{
				g_openList.UpdateNode(next);
			}
			else
			{
				g_openList.InsertData(&next);

				openAdded[nY][nX] = true;
			}
		}
	}

	return false;
}
