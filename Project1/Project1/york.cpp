#include "york.h"

void york::bark() const
{
	std::cout << "���� ����" << std::endl;
}

void york::walk() const
{
	std::cout << "���� ����" << std::endl;
}

void york::test()
{
	memset(this, 0, sizeof(york));
}
