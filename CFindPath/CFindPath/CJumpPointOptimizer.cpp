#include "CJumpPointOptimizer.h"
#include "CRayCast.h"

extern procademy::TileType g_Map[MAP_HEIGHT][MAP_WIDTH];

procademy::CJumpPointOptimizer::CJumpPointOptimizer()
{
}

procademy::CJumpPointOptimizer::~CJumpPointOptimizer()
{
	Clear();
}

void procademy::CJumpPointOptimizer::InsertNode(Node* node)
{
	Node* pNode = mNodePool.Alloc();
	pNode->g = node->g;
	pNode->h = node->h;
	pNode->f = node->f;
	pNode->position = node->position;
	pNode->dir = node->dir;
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

void procademy::CJumpPointOptimizer::Clear()
{
	Node* cur = mHead;

	while (cur != nullptr)
	{
		Node* temp = cur;
		cur = cur->pParent;
		mNodePool.Free(temp);
	}

	mHead = nullptr;
	mTail = nullptr;
	mEnd = nullptr;
	mBegin = nullptr;
	mSize = 0;
}

void procademy::CJumpPointOptimizer::Optimize()
{
	CRayCast rayCast;
	Node* temp = mBegin->pParent;

	rayCast.SetBegin(mBegin->position);
	rayCast.SetEnd(mEnd->position);

	Coordi cur;

	// end 지점까지 반복
	while (!rayCast.GetNext(cur))
	{
		if (g_Map[cur.y][cur.x] == TileType::TILE_TYPE_WALL)
		{
			mBegin = temp;

			return;
		}
	}

	// 직선 거리 경로에 벽이 없는 경우
	mBegin->pParent = mEnd;

	mNodePool.Free(temp);

	mSize--;
}
