#pragma once

#define MAP_WIDTH (100)
#define MAP_HEIGHT (50)

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

struct Node
{
	Coordi position;
	float g;
	float h;
	float f;

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

bool SearchDestination(Coordi begin, Coordi end);