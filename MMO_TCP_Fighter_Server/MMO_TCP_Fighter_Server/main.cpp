#include <locale>
#include <wtypes.h>
#include "GameProcess.h"
#include "NetworkProcs.h"
#include "CLogger.h"
#include "FrameSkip.h"
#include "MyProfiler.h"

bool g_Shutdown = false;
CLogger g_Logger;
FrameSkip gFrameSkipper;
int test = 0;

int main()
{
	setlocale(LC_ALL, "");
	// LoadData();
	CreateServer();
	srand((unsigned int)time(NULL));
	InitializeGame();

	while (!g_Shutdown)
	{
		ProfileBegin(L"Test");
		test++;
		ProfileEnd(L"Test");
		gFrameSkipper.AddLoopCounter();
		ProfileBegin(L"NetworkProc");
		NetWorkProc();
		ProfileEnd(L"NetworkProc");
		ProfileBegin(L"Logic");
		UpdateGame();
		ProfileEnd(L"Logic");
		ServerControl();
		// Monitor();
	}

	return 0;
}