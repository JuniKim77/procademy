#include "myList.h"
#include <stdio.h>
#include <stdlib.h>

class CPlayer
{
public:
	CPlayer() {}
	CPlayer(int x, int y)
		: _x(x)
		, _y(y)
	{}

	int _x = 1;
	int _y = 1;
};

int main()
{
	myList<int> ListInt;

	ListInt.push_front(1);
	ListInt.push_front(2);
	ListInt.push_front(3);
	ListInt.push_front(4);
	ListInt.push_front(5);
	ListInt.push_front(6);

	myList<int>::iterator iter;
	for (iter = ListInt.begin(); iter != ListInt.end(); ++iter)
	{
		printf("%d\n", *iter);
	}

	myList<CPlayer*> ListPlayer;

	ListPlayer.push_back(new CPlayer(1,1));
	ListPlayer.push_back(new CPlayer(2, 2));
	ListPlayer.push_back(new CPlayer(3, 3));
	ListPlayer.push_back(new CPlayer(4, 4));

	for (myList<CPlayer*>::iterator iter = ListPlayer.begin(); iter != ListPlayer.end(); ++iter)
	{
		CPlayer* p = *iter;
		printf("%d %d\n", p->_x, p->_y);
	}

	return 0;
}