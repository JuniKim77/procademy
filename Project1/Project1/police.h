#pragma once
#include <iostream>

class gun;
class police
{
	friend class gun;
public:
	police (int x, int y)
		: mX(x)
		, mY(y)
		, mpGun(nullptr)
	{}
	void movingShot();
	void equipGun(gun* gun);

private:
	int mX;
	int mY;
	gun* mpGun;
};