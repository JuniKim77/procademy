#pragma once

#define MAP_WIDTH (50)
#define MAP_HEIGHT (24)
#define CELL_SIZE (32)
#define FONT_HEIGHT (8)

#define MAX_SEARCH (50)

#include <wtypes.h>

enum class TileType
{
	TILE_TYPE_PATH,
	TILE_TYPE_WALL,
	TILE_TYPE_BEGIN,
	TILE_TYPE_END,
};

enum class NodeDirection
{
	NODE_DIRECTION_RR,
	NODE_DIRECTION_RD,
	NODE_DIRECTION_DD,
	NODE_DIRECTION_LD,
	NODE_DIRECTION_LL,
	NODE_DIRECTION_LU,
	NODE_DIRECTION_UU,
	NODE_DIRECTION_RU,
	NODE_DIRECTION_NONE,
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

struct Node
{
	Coordi position;
	float g;
	float h;
	float f;
	NodeDirection dir;
	Node* pParent;

	Node()
		: position({ 0,0 })
		, g(0)
		, h(0)
		, f(0)
		, dir(NodeDirection::NODE_DIRECTION_NONE)
		, pParent(nullptr)
	{}
	Node(Coordi _pos,
		float _g,
		float _h,
		float _f,
		NodeDirection _dir)
		: position(_pos)
		, g(_g)
		, h(_h)
		, f(_f)
		, dir(_dir)
		, pParent(nullptr)
	{}
};
/// <summary>
/// ��ġ ����
/// </summary>
/// <param name="begin"></param>
/// <param name="end"></param>
/// <param name="hdc"></param>
/// <returns></returns>
bool JumpPointSearch(Coordi begin, Coordi end, HDC hdc);
/// <summary>
/// ���� ��ġ & �ڳ� �߽߰� ��� ���� �� �߰�
/// </summary>
/// <param name="pParent"></param>
/// <param name="end"></param>
/// <param name="hdc"></param>
/// <param name="dir"></param>
/// <returns>��� ���� ����</returns>
bool SearchDirection(Node* pParent, Coordi end, HDC hdc, NodeDirection dir, HBRUSH brush);
void DrawPath(Node* end, HDC hdc);
/// <summary>
/// ������ ������ ��� ��� ����
/// ���� ����Ʈ�� ���� �ܼ��� size �� 0����
/// </summary>
void FreeNode();
void InsertNode(Node* node, HDC hdc);
HBRUSH GetColor();
void colorSet();
