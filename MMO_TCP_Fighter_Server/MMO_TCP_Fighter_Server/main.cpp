#include <locale>
#include <wtypes.h>
#include "GameProcess.h"
#include "NetworkProcs.h"
#include "CLogger.h"

bool g_Shutdown = false;
CLogger g_Logger;

int main()
{
	setlocale(LC_ALL, "");
	// LoadData();
	CreateServer();

	while (!g_Shutdown)
	{
		NetWorkProc();
		UpdateGame();
		ServerControl();
		Monitor();
	}

	return 0;
}