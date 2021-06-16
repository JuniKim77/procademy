#include "JumpPoint.h"
#include "MyHeap.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "RingBuffer.h"

#define G_Weight (1.5f)
#define H_Weight (1.0f)
#define Reverse_Stand (2.0f)

extern TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
MyHeap g_openList(128);
RingBuffer g_NodeRing;
bool g_Visit[MAP_HEIGHT][MAP_WIDTH];

extern void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
extern void DrawPoint(int x, int y, HBRUSH brush, HDC hdc);
extern HBRUSH g_Red; // µµÂø
extern HBRUSH g_Green; // ½ÃÀÛ
extern HBRUSH g_Yellow;
extern HBRUSH g_Blue;
extern HFONT g_font;
extern HPEN g_arrow;

bool JumpPointSearch(Coordi begin, Coordi end, HDC hdc)
{
	memset(g_Visit, 0, sizeof(g_Visit));

	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node* node = new Node(begin, 0, H_Weight * h, H_Weight * h, NodeDirection::NODE_DIRECTION_NONE);
	g_openList.InsertData(node);
	g_Visit[begin.y][begin.x] = true;

	while (g_openList.GetSize() > 0)
	{
		Node* cur = g_openList.GetMin();

		if (cur->position == end)
		{
			return true;
		}

		// Å½»ö ¹æÇâÀ» °áÁ¤ Áþ´Â ºÐ±â¹®
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			}
			++nY;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			}
			nY++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			nX--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX--;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			}
			++nX;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			nY++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			nX++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX++;

			if (nY < MAP_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			++nY;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			}
			nY--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			nX++;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX++;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			++nX;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);
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
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			}
			nY--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			nX--;
			//SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX--;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_NONE:
		{
			int nY = cur->position.y;
			int nX = cur->position.x + 1;

			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			nX--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY--;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			nX++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX++;
			SearchDirection(cur, end, hdc, NodeDirection::NODE_DIRECTION_RU);

			break;
		}
		default:
			break;
		}
	}

	return false;
}

bool SearchDirection(Node* pParent, Coordi end, HDC hdc, NodeDirection dir)
{
	switch (dir)
	{
	case NodeDirection::NODE_DIRECTION_RR:
	{
		bool blocked = false;

		int nX = pParent->position.x + 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX >= MAP_WIDTH)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_RR);
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (blocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX - 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_RR);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (blocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX - 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_RR);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
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

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_RR);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_DD);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			nX++;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);

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
		bool blocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y + 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_DD);
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (blocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY - 1 }, g, h, f, NodeDirection::NODE_DIRECTION_DD);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (blocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY - 1 }, g, h, f, NodeDirection::NODE_DIRECTION_DD);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
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

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_DD);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_LL);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			nX--;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_LL:
	{
		bool blocked = false;

		int nX = pParent->position.x - 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX < 0)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_LL);
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (blocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX + 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_LL);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (blocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX + 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_LL);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
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

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_LL);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_UU);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			nX++;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
		}
		else
		{
			delete bridge;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_UU:
	{
		bool blocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y - 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY < 0)
			{
				return false;
			}

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = new Node({ nX, nY }, g, 0, g, NodeDirection::NODE_DIRECTION_UU);
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (blocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY + 1 }, g, h, f, NodeDirection::NODE_DIRECTION_UU);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (blocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = new Node({ nX, nY + 1 }, g, h, f, NodeDirection::NODE_DIRECTION_UU);
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						blocked = true;
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

			if (g_Visit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = g;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;

			bool ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_UU);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, hdc, NodeDirection::NODE_DIRECTION_RR);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			nX++;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
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

void DrawPath(Node* end, HDC hdc)
{
	HPEN oldPen = (HPEN)SelectObject(hdc, g_arrow);
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

	FreeNode();

	SelectObject(hdc, oldPen);
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

void InsertNode(Node* node)
{
	g_openList.InsertData(node);
	g_Visit[node->position.y][node->position.x] = true;
	g_NodeRing.Enqueue(node);
}
