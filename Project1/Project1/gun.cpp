#include "gun.h"

void gun::shot()
{
	std::cout << "���� �߻��߽��ϴ�" << std::endl;
}

void gun::setPolice(police* police)
{
	mpPolice = police;
	mX = mpPolice->mX;
	mY = mpPolice->mY;
}

void gun::getPosition() const
{
	std::cout << "X : " << mX << " Y : " << mY << std::endl;
}
