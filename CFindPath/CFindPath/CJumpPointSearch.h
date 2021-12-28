#pragma once
#include "FilePathDTO.h"
#include "CSafeQueue.h"
#include "TC_LFObjectPool.h"
#include "CJumpPointOptimizer.h"

#define G_Weight (1.5f)
#define H_Weight (1.0f)
#define Reverse_Stand (2.0f)
#define MAX_SEARCH (50)

namespace procademy
{
	class MyHeap;

	class CJumpPointSearch
	{
	public:
		CJumpPointSearch();
		virtual ~CJumpPointSearch();
		Node* JumpPointSearch(Coordi& begin, Coordi& end);
		void Clear();

	private:
		bool SearchDirection(Node* pParent, Coordi& end, NodeDirection dir);
		void InsertNode(Node* node);

	private:
		MyHeap*						mOpenList;
		CSafeQueue<Node*>			mNodeQ;
		TC_LFObjectPool<Node>		mNodePool;
		CJumpPointOptimizer			mOptimizer;
		bool						mVisit[MAP_HEIGHT][MAP_WIDTH];
	};
}
