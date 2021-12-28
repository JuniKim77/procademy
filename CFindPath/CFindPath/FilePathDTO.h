#pragma once

#define MAP_WIDTH (60)
#define MAP_HEIGHT (40)

namespace procademy
{
	enum class TileType
	{
		TILE_TYPE_PATH,
		TILE_TYPE_WALL,
		TILE_TYPE_BEGIN,
		TILE_TYPE_END,
	};

	enum class NodeDirection
	{
		NODE_DIRECTION_RR,
		NODE_DIRECTION_RD,
		NODE_DIRECTION_DD,
		NODE_DIRECTION_LD,
		NODE_DIRECTION_LL,
		NODE_DIRECTION_LU,
		NODE_DIRECTION_UU,
		NODE_DIRECTION_RU,
		NODE_DIRECTION_NONE,
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
		NodeDirection dir;
		Node* pParent;

		Node()
			: position({ 0,0 })
			, g(0)
			, h(0)
			, f(0)
			, dir(NodeDirection::NODE_DIRECTION_NONE)
			, pParent(nullptr)
		{}
		Node(const Node* node)
			: position(node->position)
			, g(node->g)
			, h(node->h)
			, f(node->f)
			, dir(node->dir)
			, pParent(node->pParent)
		{}
		Node(Coordi _pos,
			float _g,
			float _h,
			float _f)
			: position(_pos)
			, g(_g)
			, h(_h)
			, f(_f)
			, dir(NodeDirection::NODE_DIRECTION_NONE)
			, pParent(nullptr)
		{}
		Node(Coordi _pos,
			float _g,
			float _h,
			float _f,
			NodeDirection _dir)
			: position(_pos)
			, g(_g)
			, h(_h)
			, f(_f)
			, dir(_dir)
			, pParent(nullptr)
		{}
	};
}