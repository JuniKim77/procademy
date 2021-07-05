#include <locale>
#include <wtypes.h>
#include "GameProcess.h"
#include "NetworkProcs.h"
#include "CLogger.h"
#include "FrameSkip.h"

bool g_Shutdown = false;
CLogger g_Logger;
FrameSkip gFrameSkipper;

int main()
{
	setlocale(LC_ALL, "");
	// LoadData();
	CreateServer();
	srand(time(NULL));
	InitializeGame();

	while (!g_Shutdown)
	{
		NetWorkProc();
		UpdateGame();
		ServerControl();
		Monitor();
	}

	return 0;
}