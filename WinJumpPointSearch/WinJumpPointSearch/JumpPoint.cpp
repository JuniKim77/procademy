#include "JumpPoint.h"
#include "MyHeap.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define G_Wegiht (1.5f)
#define H_Wegiht (1.0f)
#define Reverse_Stand (2.0f)

MyHeap g_openList(128);

extern void DrawCell(int x, int y, HBRUSH brush, HDC hdc);
extern void DrawPoint(int x, int y, HBRUSH brush, HDC hdc);
extern HBRUSH g_Red; // µµÂø
extern HBRUSH g_Green; // ½ÃÀÛ
extern HBRUSH g_Yellow;
extern HBRUSH g_Blue;
extern HFONT g_font;
extern HPEN g_arrow;

bool AStarSearch(Coordi begin, Coordi end, HDC hdc)
{
	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
	Node* node = new Node(begin, 0, H_Wegiht * h, H_Wegiht * h, NodeDirection::NODE_DIRECTION_NONE);
	
	

	return false;
}

bool SearchHelper(Coordi begin, Coordi end, HDC hdc, NodeDirection dir)
{
	switch (dir)
	{
	case NodeDirection::NODE_DIRECTION_RR:
		break;
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
		break;
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
