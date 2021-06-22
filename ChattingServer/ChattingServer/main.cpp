
#include <locale>
#include "NetworkProcs.h"

int main()
{
	setlocale(LC_ALL, "");

	CreateServer();

	NetWorkProc();

	return 0;
}