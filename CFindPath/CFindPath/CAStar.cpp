#include "CAStar.h"
#include "CSafeQueue.h"
#include "MyHeap.h"
#include "TC_LFObjectPool.h"


#define PriorHeight
#define MAP_WIDTH (60)
#define MAP_HEIGHT (40)

extern procademy::CAStar::TileType g_Map[MAP_HEIGHT][MAP_WIDTH];

procademy::CAStar::CAStar()
{
	mOpenList = new MyHeap(2500);
}

procademy::CAStar::~CAStar()
{
	Clear();

	delete mOpenList;
}

procademy::CAStar::Node* procademy::CAStar::SearchDestination(Coordi& begin, Coordi& end)
{
	Node* found = SearchHelper(begin, end, false);

	if (found != nullptr)
		return found;

	return SearchHelper(end, begin, true);
}

procademy::CAStar::Node* procademy::CAStar::SearchHelper(Coordi& begin, Coordi& end, bool reverse)
{
	memset(mNodeType, 0, sizeof(mNodeType));

	const int dx[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	const int dy[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	const float gWeight[8] = { 1.0, G_Wegiht, 1.0, G_Wegiht, 1.0, G_Wegiht, 1.0, G_Wegiht };

	unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);

	Node* node = mNodePool.Alloc();
	node->position = begin;
	node->g = 0;
	node->h = H_Wegiht * h;
	node->f = H_Wegiht * h;

	mOpenList->InsertData(node);
	mNodeType[begin.y][begin.x] = NodeType::NODE_TYPE_OPEN;

	int count = 0;
	int changeDirectionStand;

	if (reverse)
	{
		changeDirectionStand = h * 100.0;
	}
	else
	{
		changeDirectionStand = h * Reverse_Stand;
	}

	while (count++ < changeDirectionStand)
	{
		Node* cur = mOpenList->GetMin();

		if (cur == nullptr)
			break;

		mNodeQ.Enqueue(cur);
		mNodeType[cur->position.y][cur->position.x] = NodeType::NODE_TYPE_CLOSE;

		if (cur->position == end)
		{
			return cur;
		}

		for (int i = 0; i < 8; ++i)
		{
			int nX = cur->position.x + dx[i];
			int nY = cur->position.y + dy[i];

			if (nX < 0 || nY < 0 ||
				nX >= MAP_WIDTH || nY >= MAP_HEIGHT ||
				mNodeType[nY][nX] == NodeType::NODE_TYPE_CLOSE ||
				g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				continue;
			}

			float nG = cur->g + gWeight[i];

#ifdef PriorHeight
			float nH = H_Wegiht * (abs(nX - end.x) + abs(nY - end.y));
#else
			float nH = abs(nX - end.x) + abs(nY - end.y);
#endif
			float nF = nG + nH;

			if (mNodeType[nY][nX] == NodeType::NODE_TYPE_OPEN)
			{
				Node next({ nX, nY }, nG, nH, nF);
				next.pParent = cur;

				mOpenList->UpdateNode(&next);

				continue;
			}

			// Node* next = new Node({ nX, nY }, nG, nH, nF);
			Node* next = mNodePool.Alloc();
			next->position = { nX, nY };
			next->g = nG;
			next->h = nH;
			next->f = nF;

			next->pParent = cur;

			mOpenList->InsertData(next);

			mNodeType[nY][nX] = NodeType::NODE_TYPE_OPEN;
		}
	}

	Clear();

	return nullptr;
}

void procademy::CAStar::Clear()
{
	while (mNodeQ.IsEmpty() == false)
	{
		Node* node = mNodeQ.Dequeue();

		mNodePool.Free(node);
	}

	while (mOpenList->GetSize() > 0)
	{
		Node* node = mOpenList->GetMin();

		mNodePool.Free(node);
	}
}
