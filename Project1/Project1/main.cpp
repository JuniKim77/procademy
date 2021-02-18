#include "police.h"

int main()
{
	gun gun1;
	police police1(0, 0);

	police1.equipGun(&gun1);
	police1.movingShot();
	police1.movingShot();
	police1.movingShot();


	return 0;
}