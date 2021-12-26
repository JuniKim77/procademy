#include "JumpPoint.h"
#include "MyHeap.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "RingBuffer.h"
#include "JumpPointOptimizer.h"
#include "CRayCast.h"

#define G_Weight (1.5f)
#define H_Weight (1.0f)
#define Reverse_Stand (2.0f)

extern TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
MyHeap g_openList(1280);
RingBuffer g_NodeRing;
bool g_Visit[MAP_HEIGHT][MAP_WIDTH];
bool g_ColorTable[13][13][13];
JumpPointOptimizer g_Optimizer;

extern void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
extern void DrawPoint(int x, int y, HBRUSH brush, HDC hdc);
extern HBRUSH g_White;
extern HBRUSH g_Red; // 도착
extern HBRUSH g_Green; // 시작
extern HBRUSH g_Yellow;
extern HBRUSH g_Blue;
extern HFONT g_font;
extern HPEN g_arrow;
extern HPEN g_blueArrow;
extern bool g_space;
extern HBRUSH g_Brown;

bool JumpPointSearch(Coordi begin, Coordi end, HDC hdc)
{
	memset(g_Visit, 0, sizeof(g_Visit));
	memset(g_ColorTable, 0, sizeof(g_ColorTable));

	colorSet();

	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node* node = new Node(begin, 0, H_Weight * h, H_Weight * h, NodeDirection::NODE_DIRECTION_NONE);
	g_openList.InsertData(node);
	g_Visit[begin.y][begin.x] = true;

	while (g_openList.GetSize() > 0)
	{
		Node* cur = g_openList.GetMin();
		HBRUSH brush = GetColor();
		DrawPoint(cur->position.x, cur->position.y, brush, hdc);

		if (cur->position == end)
		{
			DrawPoint(end.x, end.y, g_Red, hdc);

			Node* temp = cur;

			while (temp != nullptr)
			{
				g_Optimizer.InsertNode(temp);

				temp = temp->pParent;
			}
			if (g_space)
			{
				DrawPathCell(g_Optimizer.GetHead(), hdc, g_Brown);
			}
			DrawPath(cur, hdc, g_arrow);
			DrawPath(g_Optimizer.GetHead(), hdc, g_blueArrow);

			DrawPoint(cur->position.x, cur->position.y, brush, hdc);

			FreeNode();
			g_Optimizer.Clear();

			return true;
		}

		// 탐색 방향을 결정 짓는 분기문
		switch (cur->dir)
		{
		case NodeDirection::NODE_DIRECTION_RR:
		{
			int nY = cur->position.y - 1;
			int nX = cur->position.x + 1;

			if (nX >= MAP_WIDTH)
			{
				break;
			}

			if (nY >= 0 && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);
			}
			++nY;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR, brush);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_RD:
		{
			int nY = cur->position.y - 1;
			int nX = cur->position.x + 1;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);
			}
			nY++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			nX--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX--;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_DD:
		{
			int nY = cur->position.y + 1;
			int nX = cur->position.x - 1;

			if (nY >= MAP_HEIGHT)
			{
				break;
			}

			if (nX >= 0 && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			}
			++nX;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD, brush);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_LD:
		{
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			}
			nY++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			nX++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX++;

			if (nY < MAP_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_LL:
		{
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nX < 0)
			{
				break;
			}

			if (nY >= 0 && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			}
			++nY;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL, brush);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_LU:
		{
			int nY = cur->position.y + 1;
			int nX = cur->position.x - 1;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			}
			nY--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			nX++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX++;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_UU:
		{
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nY < 0)
			{
				break;
			}

			if (nX >= 0 && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			}
			++nX;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU, brush);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_RU:
		{
			int nY = cur->position.y + 1;
			int nX = cur->position.x + 1;

			if (nY < FONT_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			}
			nY--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);
			nX--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX--;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_NONE:
		{
			int nY = cur->position.y;
			int nX = cur->position.x + 1;

			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR, brush);
			//nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD, brush);
			//nX--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD, brush);
			//nX--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD, brush);
			//nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL, brush);
			//nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU, brush);
			//nX++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU, brush);
			//nX++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU, brush);

			break;
		}
		default:
			break;
		}

		Sleep(30);
	}

	return false;
}

bool SearchDirection(Node* pParent, Coordi end, HDC hdc, NodeDirection dir, HBRUSH brush)
{
	switch (dir)
	{
	case NodeDirection::NODE_DIRECTION_RR:
	{
		bool upBlocked = false;
		bool downBlocked = false;

		int nX = pParent->position.x + 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX >= MAP_WIDTH)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_RR);
				node->pParent = pParent;

				InsertNode(node, hdc);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (upBlocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX - 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_RR);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						upBlocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (downBlocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX - 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_RR);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						downBlocked = true;
					}
				}
			}

			nX++;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_RD:
	{
		int nX = pParent->position.x + 1;
		int nY = pParent->position.y + 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = new Node;

		bridge->dir = NodeDirection::NODE_DIRECTION_RD;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT || nX >= MAP_WIDTH)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge, hdc);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_RR, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_DD, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if (nX >= 1 && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX + 1) < MAP_WIDTH && (nY - 1) >= 0)
			{
				if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX++;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge, hdc);

			return true;
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_DD:
	{
		bool leftBlocked = false;
		bool rightBlocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y + 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_DD);
				node->pParent = pParent;

				InsertNode(node, hdc);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (leftBlocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY - 1 }, g, h, f, NodeDirection::NODE_DIRECTION_DD);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						leftBlocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (rightBlocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY - 1 }, g, h, f, NodeDirection::NODE_DIRECTION_DD);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						rightBlocked = true;
					}
				}
			}

			nY++;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_LD:
	{
		int nX = pParent->position.x - 1;
		int nY = pParent->position.y + 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = new Node;

		bridge->dir = NodeDirection::NODE_DIRECTION_LD;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT || nX < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge, hdc);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_DD, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_LL, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX + 1) < MAP_WIDTH && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX - 1) >= 0 && (nY - 1) >= 0)
			{
				if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX--;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge, hdc);
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_LL:
	{
		bool upBlocked = false;
		bool downBlocked = false;

		int nX = pParent->position.x - 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_LL);
				node->pParent = pParent;

				InsertNode(node, hdc);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (upBlocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX + 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_LL);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						upBlocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (downBlocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX + 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_LL);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						downBlocked = true;
					}
				}
			}

			nX--;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_LU:
	{
		int nX = pParent->position.x - 1;
		int nY = pParent->position.y - 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = new Node;

		bridge->dir = NodeDirection::NODE_DIRECTION_LU;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY < 0 || nX < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge, hdc);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_LL, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_UU, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX - 1) >= 0 && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX + 1) < MAP_WIDTH && (nY - 1) >= 0)
			{
				if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX--;
			nY--;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge, hdc);
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_UU:
	{
		bool leftBlocked = false;
		bool rightBlocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y - 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_UU);
				node->pParent = pParent;

				InsertNode(node, hdc);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (leftBlocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY + 1 }, g, h, f, NodeDirection::NODE_DIRECTION_UU);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						leftBlocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (rightBlocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY + 1 }, g, h, f, NodeDirection::NODE_DIRECTION_UU);
						node->pParent = pParent;
						DrawCell(nX, nY, g_White, hdc);

						InsertNode(node, hdc);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						rightBlocked = true;
					}
				}
			}

			nY--;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_RU:
	{
		int nX = pParent->position.x + 1;
		int nY = pParent->position.y - 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = new Node;

		bridge->dir = NodeDirection::NODE_DIRECTION_RU;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY < 0 || nX >= MAP_WIDTH)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			DrawCell(nX, nY, brush, hdc);

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge, hdc);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_UU, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_RR, brush);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX + 1) < MAP_WIDTH && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX - 1) >= 0 && (nY - 1) >= 0)
			{
				if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX++;
			nY--;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge, hdc);
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_NONE:
	default:
		break;
	}

	return false;
}

void DrawPath(Node* end, HDC hdc, HPEN pen)
{
	HPEN oldPen = (HPEN)SelectObject(hdc, pen);
	MoveToEx(hdc, end->position.x * CELL_SIZE + CELL_SIZE / 2, end->position.y * CELL_SIZE + CELL_SIZE / 2, NULL);

	Node* next = end->pParent;

	while (1)
	{
		LineTo(hdc, next->position.x * CELL_SIZE + CELL_SIZE / 2, next->position.y * CELL_SIZE + CELL_SIZE / 2);

		if (next->pParent == nullptr)
		{
			break;
		}

		next = next->pParent;
	}

	SelectObject(hdc, oldPen);
}

void DrawPathCell(Node* end, HDC hdc, HBRUSH brush)
{
	Node* beginPos = end;
	Node* endPos = end->pParent;

	CRayCast rayCast;

	while (endPos != nullptr)
	{
		rayCast.SetBegin(beginPos->position);
		rayCast.SetEnd(endPos->position);

		Coordi cur;

		while (!rayCast.GetNext(cur))
		{
			DrawCell(cur.x, cur.y, brush, hdc);
		}

		rayCast.Reset();

		beginPos = beginPos->pParent;
		endPos = endPos->pParent;
	}
}

void FreeNode()
{
	g_openList.ClearHeap();

	while (g_NodeRing.GetUseSize() > 0)
	{
		Node* node;
		g_NodeRing.Dequeue(&node);

		delete node;
	}
}

void InsertNode(Node* node, HDC hdc)
{
	g_openList.InsertData(node);
	g_Visit[node->position.y][node->position.x] = true;
	g_NodeRing.Enqueue(node);

	DrawCell(node->position.x, node->position.y, g_Yellow, hdc);
}

HBRUSH GetColor()
{
	while (1)
	{
		int r = (rand() & 0xff) / 20;
		int g = (rand() & 0xff) / 20;
		int b = (rand() & 0xff) / 20;

		if (g_ColorTable[r][g][b] == false)
		{
			g_ColorTable[r][g][b] == true;

			return CreateSolidBrush(RGB(r * 20, g * 20, b * 20));
		}
	}

	return HBRUSH();
}

void colorSet()
{
	for (int i = 12; i >= 10; --i)
	{
		for (int j = 12; j >= 10; --j)
		{
			for (int k = 12; k >= 10; --k)
			{
				g_ColorTable[i][j][k] = true;
			}
		}
	}
}
