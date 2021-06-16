#include "JumpPoint.h"
#include "MyHeap.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "RingBuffer.h"

#define G_Wegiht (1.5f)
#define H_Wegiht (1.0f)
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
	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node* node = new Node(begin, 0, H_Wegiht * h, H_Wegiht * h, NodeDirection::NODE_DIRECTION_NONE);
	g_openList.InsertData(node);

	while (g_openList.GetSize() > 0)
	{
		Node* cur = g_openList.GetMin();

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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			}
			++nY;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			}
			nY++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			nX--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX--;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			}
			++nX;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			nY++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			nX++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX++;

			if (nY < MAP_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			++nY;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			}
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			nX++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX++;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}
			++nX;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);
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
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			}
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);
			nX--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX--;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_NONE:
		{
			int nY = cur->position.y;
			int nX = cur->position.x + 1;

			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RR);
			nY++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RD);
			nX--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_DD);
			nX--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LD);
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LL);
			nY--;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_LU);
			nX++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_UU);
			nX++;
			SearchDirection({ nX, nY }, end, hdc, NodeDirection::NODE_DIRECTION_RU);

			break;
		}
		default:
			break;
		}
	}

	return false;
}

bool SearchDirection(Coordi begin, Coordi end, HDC hdc, NodeDirection dir)
{
	switch (dir)
	{
	case NodeDirection::NODE_DIRECTION_RR:
	{
		bool blocked = false;

		int nX = begin.x;
		int nY = begin.y;
		int count = 0;

		while (count < MAP_WIDTH)
		{
			if (nX >= MAP_WIDTH)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				return true;
			}

			if (nY - 1 >= 0)
			{
				if (blocked == true && g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
				{
					int g = nX - begin.x + 1;
					int h = abs(nX - end.x) + abs(nY - end.y);
					float f = g + h * H_Wegiht;

					Node* node = new Node({ nX - 1, nY }, g, h, f, NodeDirection::NODE_DIRECTION_RR);

					return false;
				}

				if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
				{
					blocked = true;
				}
			}

			count++;
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_RD:
		break;
	case NodeDirection::NODE_DIRECTION_DD:
		break;
	case NodeDirection::NODE_DIRECTION_LD:
		break;
	case NodeDirection::NODE_DIRECTION_LL:
		break;
	case NodeDirection::NODE_DIRECTION_LU:
		break;
	case NodeDirection::NODE_DIRECTION_UU:
		break;
	case NodeDirection::NODE_DIRECTION_RU:
		break;
	case NodeDirection::NODE_DIRECTION_NONE:
	default:
		break;
	}

	return true;
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
}
