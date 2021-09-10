#include "AStar.h"
#include "MyHeap.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "RingBuffer.h"

#define PriorHeight
//#define LogCell
#define G_Wegiht (1.5f)
#define H_Wegiht (2.0f)
#define Reverse_Stand (2.0f)

extern TileType g_Map[MAP_HEIGHT][MAP_WIDTH];
MyHeap g_openList(128);
RingBuffer g_NodeRing;
NodeType g_NodeType[MAP_HEIGHT][MAP_WIDTH];

extern void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
extern void DrawPoint(int x, int y, HBRUSH brush, HDC hdc);
extern HBRUSH g_Red; // µµÂø
extern HBRUSH g_Green; // ½ÃÀÛ
extern HBRUSH g_Yellow;
extern HBRUSH g_Blue;
extern HFONT g_font;
extern HPEN g_arrow;

bool SearchDestination(Coordi begin, Coordi end, HDC hdc)
{
	bool found = SearchHelper(begin, end, hdc, false);

	if (found) 
		return true;

	return SearchHelper(end, begin, hdc, true);
}

bool SearchHelper(Coordi begin, Coordi end, HDC hdc, bool reverse)
{
	memset(g_NodeType, 0, sizeof(g_NodeType));

	const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const float gWeight[8] = { 1.0, G_Wegiht, 1.0, G_Wegiht, 1.0, G_Wegiht, 1.0, G_Wegiht };

	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node* node = new Node(begin, 0, H_Wegiht * h, H_Wegiht * h);
	g_openList.InsertData(node);
	g_NodeType[begin.y][begin.x] = NodeType::NODE_TYPE_OPEN;
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, g_font);

	int count = 0;
	int changeDirectionStand;

	if (reverse)
	{
		changeDirectionStand = h * 10.0;
	}
	else
	{
		changeDirectionStand = h * Reverse_Stand;
	}

	while (count++ < changeDirectionStand)
	{
		Node* cur = g_openList.GetMin();

		if (cur == nullptr)
			break;

		g_NodeRing.Enqueue(cur);
		DrawCell(cur->position.x, cur->position.y, g_Yellow, hdc);
		g_NodeType[cur->position.y][cur->position.x] = NodeType::NODE_TYPE_CLOSE;

		if (cur->position == begin)
		{
			DrawPoint(begin.x, begin.y, g_Green, hdc);
		}

#ifdef LogCell
		LogInfo(cur, hdc);
#endif

		if (cur->position == end)
		{
			// Rendoring
			DrawPoint(cur->position.x, cur->position.y, g_Red, hdc);
			DrawPath(cur, hdc);

			return true;
		}

		for (int i = 0; i < 8; ++i)
		{
			int nX = cur->position.x + dx[i];
			int nY = cur->position.y + dy[i];

			if (nX < 0 || nY < 0 ||
				nX >= MAP_WIDTH || nY >= MAP_HEIGHT ||
				g_NodeType[nY][nX] == NodeType::NODE_TYPE_CLOSE ||
				g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				continue;
			}

			float nG = cur->g + gWeight[i];
#ifdef PriorHeight
			float nH = H_Wegiht * (abs(nX - end.x) + abs(nY - end.y));
#else
			float nH = abs(nX - end.x) + abs(nY - end.y);
#endif
			float nF = nG + nH;

			if (g_NodeType[nY][nX] == NodeType::NODE_TYPE_OPEN)
			{
				Node next({ nX, nY }, nG, nH, nF);
				next.pParent = cur;

				g_openList.UpdateNode(&next);
				DrawCell(next.position.x, next.position.y, g_Blue, hdc);

#ifdef LogCell
				LogInfo(&next, hdc);
#endif

				continue;
			}

			Node* next = new Node({ nX, nY }, nG, nH, nF);

			next->pParent = cur;

			g_openList.InsertData(next);
			DrawCell(next->position.x, next->position.y, g_Blue, hdc);

#ifdef LogCell
			LogInfo(next, hdc);
#endif
			g_NodeType[nY][nX] = NodeType::NODE_TYPE_OPEN;
		}

		Sleep(30);
	}

	FreeNode();

	return count < changeDirectionStand;
}

void LogInfo(Node* cur, HDC hdc)
{
	char str[10] = { 0, };
	sprintf_s(str, "G: %.1f", cur->g);
	TextOutA(hdc, cur->position.x * CELL_SIZE, cur->position.y * CELL_SIZE, str, 10);
	sprintf_s(str, "H: %.1f", cur->h);
	TextOutA(hdc, cur->position.x * CELL_SIZE, cur->position.y * CELL_SIZE + FONT_HEIGHT, str, 10);
	sprintf_s(str, "F: %.1f", cur->f);
	TextOutA(hdc, cur->position.x * CELL_SIZE, cur->position.y * CELL_SIZE + FONT_HEIGHT * 2, str, 10);
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
	while (g_NodeRing.GetUseSize() != 0)
	{
		Node* node;
		g_NodeRing.Dequeue(&node);

		delete node;
	}

	while (g_openList.GetSize() > 0)
	{
		Node* node = g_openList.GetMin();

		delete node;
	}
}
