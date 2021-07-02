#include <locale>
#include <wtypes.h>
#include "GameProcess.h"
#include "NetworkProcs.h"

#pragma comment(lib, "winmm.lib")

bool g_Shutdown = false;

int main()
{
	setlocale(LC_ALL, "");
	timeBeginPeriod(1);
	// LoadData();
	CreateServer();

	while (!g_Shutdown)
	{
		NetWorkProc();
		UpdateGame();
		ServerControl();
		Monitor();
	}



	timeEndPeriod(1);

	return 0;
}