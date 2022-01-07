#pragma once
#include "FilePathDTO.h"
#include "TC_LFObjectPool.h"

namespace procademy
{
	class CJumpPointOptimizer
	{
	public:
		CJumpPointOptimizer();
		virtual ~CJumpPointOptimizer();
		void InsertNode(Node* node);
		void Clear();
		Node* GetHead() { return mHead; }
	private:
		void Optimize();

	private:
		int mSize = 0;
		Node* mHead = nullptr;
		Node* mTail = nullptr;
		Node* mBegin = nullptr;
		Node* mEnd = nullptr;
		TC_LFObjectPool<Node> mNodePool;
	};
}