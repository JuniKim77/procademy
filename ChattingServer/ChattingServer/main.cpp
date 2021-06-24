
#include <locale>
#include "NetworkProcs.h"

int main()
{
	setlocale(LC_ALL, "");

	CreateServer();

	while (1)
	{
		NetWorkProc();
	}

	return 0;
}