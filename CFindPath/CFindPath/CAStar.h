#pragma once
#include "FilePathDTO.h"
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

	public:
		CAStar();
		virtual ~CAStar();
		Node* SearchDestination(Coordi& begin, Coordi& end);
		int Clear();

	private:
		Node* SearchHelper(Coordi& begin, Coordi& end, bool reverse);

	private:
		MyHeap*						mOpenList = nullptr;
		CSafeQueue<Node*>			mNodeQ;
		NodeType					mNodeType[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
		TC_LFObjectPool<Node>		mNodePool;
	};
}
