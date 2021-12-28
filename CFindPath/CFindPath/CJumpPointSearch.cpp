#include "CJumpPointSearch.h"
#include "MyHeap.h"

extern procademy::TileType g_Map[MAP_HEIGHT][MAP_WIDTH];

procademy::CJumpPointSearch::CJumpPointSearch()
{
    mOpenList = new MyHeap(2500);
}

procademy::CJumpPointSearch::~CJumpPointSearch()
{
    Clear();
    delete mOpenList;
}

procademy::Node* procademy::CJumpPointSearch::JumpPointSearch(Coordi& begin, Coordi& end)
{
    memset(mVisit, 0, sizeof(mVisit));

    unsigned short h = abs(begin.x - end.x) + abs(begin.y - end.y);
    Node* node = mNodePool.Alloc();
    node->position = begin;
    node->g = 0;
    node->h = H_Weight * h;
    node->f = H_Weight * h;
    node->dir = NodeDirection::NODE_DIRECTION_NONE;
	node->pParent = nullptr;
    mOpenList->InsertData(node);
    mVisit[begin.y][begin.x] = true;

    while (mOpenList->GetSize() > 0)
    {
		Node* cur = mOpenList->GetMin();

		if (cur->position == end)
		{
			Node* temp = cur;

			while (temp != nullptr)
			{
				mOptimizer.InsertNode(temp);

				temp = temp->pParent;
			}

			mNodeQ.Enqueue(cur);

			return mOptimizer.GetHead();
		}

		// 탐색 방향을 결정 짓는 분기문
		switch (cur->dir)
		{
		case NodeDirection::NODE_DIRECTION_RR:
		{
			//	[o]  [x]
			// cur  []
			//	[o]	[x]
			int nY = cur->position.y - 1;
			int nX = cur->position.x + 1;

			if (nX >= MAP_WIDTH)
			{
				break;
			}

			if (nY >= 0 && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);
			}
			++nY;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RR);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_RD:
		{
			//		[o]  [x]
			//	[o]	cur  []
			//	[x]	[]	[x]
			int nY = cur->position.y - 1;
			int nX = cur->position.x + 1;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);
			}
			nY++;
			nY++;
			if (nY < MAP_HEIGHT && nX < MAP_WIDTH)
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			nX--;
			nX--;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_DD:
		{
			//	[o]	cur	[o]
			//	[x]	[]	[x]
			int nY = cur->position.y + 1;
			int nX = cur->position.x - 1;

			if (nY >= MAP_HEIGHT)
			{
				break;
			}

			if (nX >= 0 && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			}
			++nX;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_DD);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_LD:
		{
			//	[x]	[o]
			//	[]	cur [o]
			//	[x]	[]	[x]
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			}
			nY++;
			nY++;
			if (nY < MAP_HEIGHT && nX >= 0)
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			nX++;
			nX++;

			if (nY < MAP_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_LL:
		{
			//	[x] [o]
			//	[]  cur
			//	[x]	[o]
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nX < 0)
			{
				break;
			}

			if (nY >= 0 && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			}
			++nY;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LL);
			++nY;
			if (nY < MAP_HEIGHT && g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_LU:
		{
			//	[x]	[]	[x]
			//	[]	cur [o]
			//	[x]	[o]	
			int nY = cur->position.y + 1;
			int nX = cur->position.x - 1;

			if (nY < MAP_HEIGHT && nX >= 0 &&
				g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			}
			nY--;
			nY--;
			if (nY >= 0 && nX >= 0)
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			nX++;
			nX++;

			if (nY >= 0 && nX < MAP_WIDTH &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_UU:
		{
			//	[x]	[]	[x]
			//	[o]	cur	[o]
			int nY = cur->position.y - 1;
			int nX = cur->position.x - 1;

			if (nY < 0)
			{
				break;
			}

			if (nX >= 0 && g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			}
			++nX;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_UU);
			++nX;
			if (nX < MAP_WIDTH && g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL
				&& g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);
			}
			break;
		}
		case NodeDirection::NODE_DIRECTION_RU:
		{
			//	[x]	[]	[x]
			//	[o]	cur []
			//		[o]	[x]
			int nY = cur->position.y + 1;
			int nX = cur->position.x + 1;

			if (nY < MAP_HEIGHT && nX < MAP_WIDTH &&
				g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			}
			nY--;
			nY--;
			if (nY >= 0 && nX < MAP_WIDTH)
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);
			nX--;
			nX--;

			if (nY >= 0 && nX >= 0 &&
				g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
				g_Map[nY][nX] == TileType::TILE_TYPE_PATH)
			{
				SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			}

			break;
		}
		case NodeDirection::NODE_DIRECTION_NONE:
		{
			int nY = cur->position.y;
			int nX = cur->position.x + 1;

			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RR);
			//nY++;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RD);
			//nX--;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_DD);
			//nX--;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LD);
			//nY--;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LL);
			//nY--;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_LU);
			//nX++;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_UU);
			//nX++;
			SearchDirection(cur, end, NodeDirection::NODE_DIRECTION_RU);

			break;
		}
		default:
			break;
		}

		mNodeQ.Enqueue(cur);
    }

    return nullptr;
}

bool procademy::CJumpPointSearch::SearchDirection(Node* pParent, Coordi& end, NodeDirection dir)
{
	switch (dir)
	{
	case NodeDirection::NODE_DIRECTION_RR:
	{
		bool upBlocked = false;
		bool downBlocked = false;

		int nX = pParent->position.x + 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX >= MAP_WIDTH)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (mVisit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = mNodePool.Alloc();
				node->position = { nX, nY };
				node->g = g;
				node->f = 0;
				node->h = g;
				node->dir = NodeDirection::NODE_DIRECTION_RR;
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (upBlocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX - 1, nY };
						node->g = g;
						node->f = h;
						node->h = f;
						node->dir = NodeDirection::NODE_DIRECTION_RR;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						upBlocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (downBlocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX - 1, nY };
						node->g = g;
						node->f = h;
						node->h = f;
						node->dir = NodeDirection::NODE_DIRECTION_RR;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						downBlocked = true;
					}
				}
			}

			nX++;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_RD:
	{
		int nX = pParent->position.x + 1;
		int nY = pParent->position.y + 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = mNodePool.Alloc();

		bridge->dir = NodeDirection::NODE_DIRECTION_RD;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT || nX >= MAP_WIDTH)
			{
				break;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				break;
			}

			if (mVisit[nY][nX] == true)
			{
				break;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = 0;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_RR);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_DD);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if (nX >= 1 && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX + 1) < MAP_WIDTH && (nY - 1) >= 0)
			{
				if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX++;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);

			return true;
		}
		else
		{
			//mNodePool.Free(bridge);
			mNodeQ.Enqueue(bridge);
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_DD:
	{
		bool leftBlocked = false;
		bool rightBlocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y + 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (mVisit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = mNodePool.Alloc();
				node->position = { nX, nY };
				node->g = g;
				node->h = 0;
				node->f = 0;
				node->dir = NodeDirection::NODE_DIRECTION_DD;
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (leftBlocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX, nY - 1 };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_DD;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						leftBlocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (rightBlocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY - 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX, nY - 1 };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_DD;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						rightBlocked = true;
					}
				}
			}

			nY++;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_LD:
	{
		int nX = pParent->position.x - 1;
		int nY = pParent->position.y + 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = mNodePool.Alloc();

		bridge->dir = NodeDirection::NODE_DIRECTION_LD;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY >= MAP_HEIGHT || nX < 0)
			{
				break;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				break;
			}

			if (mVisit[nY][nX] == true)
			{
				break;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = 0;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_DD);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_LL);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX + 1) < MAP_WIDTH && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX - 1) >= 0 && (nY - 1) >= 0)
			{
				if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX--;
			nY++;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
		}
		else
		{
			//mNodePool.Free(bridge);
			mNodeQ.Enqueue(bridge);
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_LL:
	{
		bool upBlocked = false;
		bool downBlocked = false;

		int nX = pParent->position.x - 1;
		int nY = pParent->position.y;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nX < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (mVisit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = mNodePool.Alloc();
				node->position = { nX, nY };
				node->g = g;
				node->h = 0;
				node->f = 0;
				node->dir = NodeDirection::NODE_DIRECTION_LL;
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nY - 1 >= 0)
			{
				if (upBlocked == true)
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX + 1, nY };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_LL;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY - 1][nX] == TileType::TILE_TYPE_WALL)
					{
						upBlocked = true;
					}
				}
			}

			if (nY + 1 < MAP_HEIGHT)
			{
				if (downBlocked == true)
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX + 1 - end.x) + abs(nY - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX + 1, nY };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_LL;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL)
					{
						downBlocked = true;
					}
				}
			}

			nX--;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_LU:
	{
		int nX = pParent->position.x - 1;
		int nY = pParent->position.y - 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = mNodePool.Alloc();

		bridge->dir = NodeDirection::NODE_DIRECTION_LU;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY < 0 || nX < 0)
			{
				break;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				break;
			}

			if (mVisit[nY][nX] == true)
			{
				break;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = 0;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_LL);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_UU);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX - 1) >= 0 && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX + 1) < MAP_WIDTH && (nY - 1) >= 0)
			{
				if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX--;
			nY--;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
		}
		else
		{
			//mNodePool.Free(bridge);
			mNodeQ.Enqueue(bridge);
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_UU:
	{
		bool leftBlocked = false;
		bool rightBlocked = false;

		int nX = pParent->position.x;
		int nY = pParent->position.y - 1;
		int count = 0;

		while (count < MAX_SEARCH)
		{
			if (nY < 0)
			{
				return false;
			}

			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				return false;
			}

			if (mVisit[nY][nX] == true)
			{
				return false;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + count + 1;

				Node* node = mNodePool.Alloc();
				node->position = { nX, nY };
				node->g = g;
				node->h = 0;
				node->f = 0;
				node->dir = NodeDirection::NODE_DIRECTION_UU;
				node->pParent = pParent;

				InsertNode(node);

				return true;
			}

			if (nX - 1 >= 0)
			{
				if (leftBlocked == true)
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX, nY + 1 };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_UU;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL)
					{
						leftBlocked = true;
					}
				}
			}

			if (nX + 1 < MAP_HEIGHT)
			{
				if (rightBlocked == true)
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_PATH)
					{
						int g = pParent->g + count;
						int h = abs(nX - end.x) + abs(nY + 1 - end.y);
						float f = g + h * H_Weight;

						Node* node = mNodePool.Alloc();
						node->position = { nX, nY + 1 };
						node->g = g;
						node->h = h;
						node->f = f;
						node->dir = NodeDirection::NODE_DIRECTION_UU;
						node->pParent = pParent;

						InsertNode(node);

						return true;
					}
				}
				else
				{
					if (g_Map[nY][nX + 1] == TileType::TILE_TYPE_WALL)
					{
						rightBlocked = true;
					}
				}
			}

			nY--;
			count++;
		}
		break;
	}
	case NodeDirection::NODE_DIRECTION_RU:
	{
		int nX = pParent->position.x + 1;
		int nY = pParent->position.y - 1;
		int count = 0;
		bool hasCreated = false;
		Node* bridge = mNodePool.Alloc();

		bridge->dir = NodeDirection::NODE_DIRECTION_RU;
		bridge->pParent = pParent;

		while (count < MAX_SEARCH)
		{
			if (nY < 0 || nX >= MAP_WIDTH)
			{
				break;
			}
			if (g_Map[nY][nX] == TileType::TILE_TYPE_WALL)
			{
				break;
			}

			if (mVisit[nY][nX] == true)
			{
				break;
			}

			if (end.x == nX && end.y == nY)
			{
				int g = pParent->g + (count + 1) * G_Weight;

				bridge->g = g;
				bridge->h = 0;
				bridge->f = 0;
				bridge->position = { nX, nY };

				InsertNode(bridge);

				return true;
			}

			int g = pParent->g + (count + 1) * G_Weight;
			int h = abs(nX - end.x) + abs(nY - end.y);
			float f = g + h * H_Weight;

			bridge->g = g;
			bridge->h = h;
			bridge->f = f;
			bridge->position = { nX, nY };

			bool ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_UU);

			if (ret == true)
			{
				hasCreated = true;
			}

			ret = SearchDirection(bridge, end, NodeDirection::NODE_DIRECTION_RR);

			if (ret == true)
			{
				hasCreated = true;
			}

			if (hasCreated)
			{
				break;
			}

			// 코너 체크
			if ((nX + 1) < MAP_WIDTH && (nY + 1) < MAP_HEIGHT)
			{
				if (g_Map[nY + 1][nX] == TileType::TILE_TYPE_WALL &&
					g_Map[nY + 1][nX + 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			if ((nX - 1) >= 0 && (nY - 1) >= 0)
			{
				if (g_Map[nY][nX - 1] == TileType::TILE_TYPE_WALL &&
					g_Map[nY - 1][nX - 1] == TileType::TILE_TYPE_PATH)
				{
					hasCreated = true;
					break;
				}
			}

			nX++;
			nY--;
			count++;
		}

		if (hasCreated)
		{
			bridge->position = { nX, nY };

			InsertNode(bridge);
		}
		else
		{
			//mNodePool.Free(bridge);
			mNodeQ.Enqueue(bridge);
		}

		break;
	}
	case NodeDirection::NODE_DIRECTION_NONE:
	default:
		break;
	}

	return false;
}

void procademy::CJumpPointSearch::InsertNode(Node* node)
{
	mOpenList->InsertData(node);
	mVisit[node->position.y][node->position.x] = true;
	//mNodeQ.Enqueue(node);
}

void procademy::CJumpPointSearch::Clear()
{
	while (mNodeQ.IsEmpty() == false)
	{
		Node* node = mNodeQ.Dequeue();

		if (node != nullptr)
			mNodePool.Free(node);
	}

	while (mOpenList->GetSize() > 0)
	{
		Node* node = mOpenList->GetMin();

		if (node != nullptr)
			mNodePool.Free(node);
	}

	mOptimizer.Clear();
}
