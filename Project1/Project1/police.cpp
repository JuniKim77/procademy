#include "police.h"
#include "gun.h"

void police::movingShot()
{
	std::cout << "���� ��ġ(x, y) : " << mX << " " << mY << std::endl;
	mX++;
	mY++;
	std::cout << "���� ��ġ(x, y) : " << mX << " " << mY << std::endl;
	mpGun->shot();
	mpGun->getPosition();
}

void police::equipGun(gun* gun)
{
	std::cout << "���� �����Ͽ����ϴ�." << std::endl;
	mpGun = gun;
	mpGun->setPolice(this);
}
