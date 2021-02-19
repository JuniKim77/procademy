#include "police.h"
#include "gun.h"

void police::movingShot()
{
	std::cout << "현재 위치(x, y) : " << mX << " " << mY << std::endl;
	mX++;
	mY++;
	std::cout << "나중 위치(x, y) : " << mX << " " << mY << std::endl;
	mpGun->shot();
	mpGun->getPosition();
}

void police::equipGun(gun* gun)
{
	std::cout << "총을 장착하였습니다." << std::endl;
	mpGun = gun;
	mpGun->setPolice(this);
}
