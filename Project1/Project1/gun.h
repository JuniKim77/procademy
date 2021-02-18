#pragma once
#include <iostream>
#include "police.h"

class police;
class gun
{
public:
	void shot();
	void setPolice(police* police);
	void getPosition() const;

private:
	int mX = 0;
	int mY = 0;
	police* mpPolice = nullptr;
};