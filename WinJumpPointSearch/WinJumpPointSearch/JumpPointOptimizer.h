#pragma once
#include "JumpPoint.h"

class JumpPointOptimizer
{
public:
	JumpPointOptimizer();
	~JumpPointOptimizer();
	void InsertNode(Node* node);
	void Clear();
	Node* GetHead() { return mHead; }
	
private:
	void Optimize();

private:
	int mSize;
	Node* mHead;
	Node* mTail;
	Node* mBegin;
	Node* mEnd;
};