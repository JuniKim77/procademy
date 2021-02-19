#include "gun.h"
#include "police.h"
#include "york.h"

int main()
{
	gun gun1;
	police police1(0, 0);

	police1.equipGun(&gun1);
	police1.movingShot();
	police1.movingShot();
	police1.movingShot();

	animal* a = new animal();
	animal* b = new york();
	animal* c = new dog();

	a->bark();
	b->bark();
	c->bark();

	a->walk();
	b->walk();
	c->walk();
	
	a->run();
	b->run();
	c->run();

	york* d = (york*)b;
	d->test();

	delete a;
	delete b;
	delete c;

	return 0;
}