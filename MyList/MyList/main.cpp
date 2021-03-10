#pragma once
#include "myList.h"
#include <stdio.h>
#include <stdlib.h>
#include "operatorNewOverload.h"

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

	ListInt.clear();

	myList<CPlayer*> ListPlayer;

	CPlayer* p1 = new CPlayer(1, 1);
	CPlayer* p2 = new CPlayer(2, 2);
	CPlayer* p3 = new CPlayer(3, 3);
	CPlayer* p4 = new CPlayer(4, 4);

	ListPlayer.push_back(p1);
	ListPlayer.push_back(p2);
	ListPlayer.push_back(p3);
	ListPlayer.push_back(p4);

	for (myList<CPlayer*>::iterator iter = ListPlayer.begin(); iter != ListPlayer.end(); ++iter)
	{
		CPlayer* p = *iter;
		printf("%d %d\n", p->_x, p->_y);
	}

	CPlayer* cur = ListPlayer.pop_front();
	delete cur;

	for (auto iter = ListPlayer.begin(); iter != ListPlayer.end(); )
	{
		delete *iter;
		iter = ListPlayer.erase(iter);
	}

	return 0;
}