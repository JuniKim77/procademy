#pragma once

#include "CSafeQueue.h"
#include "TC_LFObjectPool.h"

#define G_Wegiht (1.5f)
#define H_Wegiht (2.0f)
#define Reverse_Stand (2.0f)

namespace procademy
{
	class MyHeap;

	class CAStar
	{
	public:
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

		enum {
			DEFAULT_HEAP_SIZE = 1000,
			MAP_MAX_HEIGHT = 500,
			MAP_MAX_WIDTH = 500,
		};

		struct Node
		{
			Coordi position;
			float g;	// 출발점으로부터의 거리
			float h;	// 목적지까지 거리
			float f;	// g + h

			Node* pParent;

			Node()
				: position({ 0,0 })
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

	public:
		CAStar();
		virtual ~CAStar();
		Node* SearchDestination(Coordi& begin, Coordi& end);
		void Clear();


	private:
		Node* SearchHelper(Coordi& begin, Coordi& end, bool reverse);

	private:
		MyHeap*						mOpenList = nullptr;
		CSafeQueue<Node*>			mNodeQ;
		NodeType					mNodeType[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
		TC_LFObjectPool<Node>		mNodePool;
	};
}
