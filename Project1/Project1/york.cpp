#include "york.h"

void york::bark() const
{
	std::cout << "ÄÈÄÈ ÄÈÄÈ" << std::endl;
}

void york::walk() const
{
	std::cout << "ÃÑÃÑ ÃÑÃÑ" << std::endl;
}

void york::test()
{
	memset(this, 0, sizeof(york));
}
