#pragma once

#define MAP_WIDTH (50)
#define MAP_HEIGHT (24)
#define CELL_SIZE (32)
#define FONT_HEIGHT (8)

#include <wtypes.h>

enum class TileType
{
	TILE_TYPE_PATH,
	TILE_TYPE_WALL,
	TILE_TYPE_BEGIN,
	TILE_TYPE_END,
};

struct Coordi
{
	int x;
	int y;

	bool operator == (Coordi& other)
	{
		return x == other.x && y == other.y;
	}
};

enum class NodeType
{
	NODE_TYPE_NONE,
	NODE_TYPE_OPEN,
	NODE_TYPE_CLOSE,
};

struct Node
{
	Coordi position;
	float g;	// 출발점으로부터의 거리
	float h;	// 목적지까지 거리
	float f;	// g + h

	Node* pParent;

	Node() 
		: position({0,0})
		, g(0)
		, h(0)
		, f(0)
		, pParent(nullptr)
	{}
	Node(Coordi _pos,
		float _g,
		float _h,
		float _f)
		: position(_pos)
		, g(_g)
		, h(_h)
		, f(_f)
		, pParent(nullptr)
	{}
};

bool SearchDestination(Coordi begin, Coordi end, HDC hdc);

bool SearchHelper(Coordi begin, Coordi end, HDC hdc, bool reverse);

void LogInfo(Node* cur, HDC hdc);

void DrawPath(Node* end, HDC hdc);

void FreeNode();