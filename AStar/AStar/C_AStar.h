#pragma once

class C_AStar
{
public:
	enum class TileType
	{
		TILE_TYPE_PATH,
		TILE_TYPE_WALL,
		TILE_TYPE_BEGIN,
		TILE_TYPE_END
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

private:



};