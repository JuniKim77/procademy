#include "JumpPointOptimizer.h"
#include "CRayCast.h"

extern TileType g_Map[MAP_HEIGHT][MAP_WIDTH];

JumpPointOptimizer::JumpPointOptimizer()
	: mHead(nullptr)
	, mTail(nullptr)
	, mBegin(nullptr)
	, mEnd(nullptr)
	, mSize(0)
{
}

JumpPointOptimizer::~JumpPointOptimizer()
{
	Clear();
}

void JumpPointOptimizer::InsertNode(Node* node)
{
	Node* pNode = new Node(node);
	pNode->pParent = nullptr;

	if (mHead == nullptr)
	{
		mHead = pNode;
		mTail = pNode;
		mEnd = pNode;
		mBegin = pNode;

		mSize = 1;

		return;
	}

	mTail->pParent = pNode;
	mTail = pNode;
	mSize++;

	mEnd = pNode;

	if (mSize > 2)
	{
		Optimize();
	}
}

void JumpPointOptimizer::Clear()
{
	Node* cur = mHead;

	while (cur != nullptr)
	{
		Node* temp = cur;
		cur = cur->pParent;
		delete temp;
	}
	mHead = nullptr;
	mTail = nullptr;
	mEnd = nullptr;
	mBegin = nullptr;
	mSize = 0;
}

void JumpPointOptimizer::Optimize()
{
	CRayCast rayCast;
	Node* temp = mBegin->pParent;

	rayCast.SetBegin(mBegin->position);
	rayCast.SetEnd(mEnd->position);

	Coordi cur;

	while (!rayCast.GetNext(cur))
	{
		if (g_Map[cur.y][cur.x] == TileType::TILE_TYPE_WALL)
		{
			mBegin = temp;

			return;
		}
	}

	mBegin->pParent = mEnd;

	delete temp;

	mSize--;
}
