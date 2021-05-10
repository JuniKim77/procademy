#include "star.h"

star::star()
	: x(WINDOW_WIDTH / 2)
	, oldX(WINDOW_WIDTH / 2)
	, y(WINDOW_HEIGHT / 2)
	, oldY(WINDOW_HEIGHT / 2)
	, ID(-1)
{
	
}

void star::move(Dir dir)
{
	int nX = x;
	int nY = y;

	switch (dir)
	{
	case Dir::DIR_UP:
		nY = y - 1;
		break;
	case Dir::DIR_DOWN:
		nY = y + 1;
		break;
	case Dir::DIR_LEFT:
		nX = x - 1;
		break;
	case Dir::DIR_RIGHT:
		nX = x + 1;
		break;
	default:
		break;
	}

	if (nX < 0 || nY < 0 || nX >= WINDOW_WIDTH || nY >= WINDOW_HEIGHT)
		return;

	x = nX;
	y = nY;
}

void star::save()
{
	oldX = x;
	oldY = y;
}
