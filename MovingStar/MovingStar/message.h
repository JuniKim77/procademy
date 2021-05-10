#pragma once

struct ID_ALLOCATE
{
	int type;
	int ID;
	int x;
	int y;
};

struct CREATE_STAR
{
	int type;
	int ID;
	int x;
	int y;
};

struct DESTROY_STAR
{
	int type;
	int ID;
	int x;
	int y;
};

struct MOVE_STAR
{
	int type;
	int ID;
	int x;
	int y;

	MOVE_STAR(int id, int _x, int _y)
		: type(3)
		, ID(id)
		, x(_x)
		, y(_y)
	{
	}
};