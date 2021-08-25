#include "GameProcess.h"
#include "FrameSkip.h"
#include "User.h"
#include <unordered_map>
#include "Container.h"
#include "Session.h"
#include "NetworkProcs.h"
#include "Content.h"
#include "CLogger.h"
#include <conio.h>
#include "MyProfiler.h"

using namespace std;

extern bool g_Shutdown;
extern unordered_map<DWORD, User*> g_users;
extern FrameSkip gFrameSkipper;
extern CLogger g_Logger;
DWORD g_DisconnectCount = 0;
DWORD g_ConnectCount = 0;
bool g_bMonitoring = false;
DWORD g_sendTPS = 0;
DWORD g_recvTPS = 0;
bool g_bMonitorTimer = false;
int g_FPS = 0;
int g_LPS = 0;

void InitializeGame()
{
	gFrameSkipper.CheckTime();
	gFrameSkipper.Reset();
	g_Logger.setLogLevel(dfLOG_LEVEL_DEBUG);
}

void UpdateGame()
{
	gFrameSkipper.CheckTime();

	if (!gFrameSkipper.IsSkip())
	{
		return;
	}

	gFrameSkipper.UpdateRemain();

	if (gFrameSkipper.GetTotalTick() >= 1000)
	{
		if (g_bMonitoring)
			g_bMonitorTimer = true;

		int frameCount = gFrameSkipper.GetFrameCount();

		if (frameCount >= 52 || frameCount <= 48)
		{
			gFrameSkipper.PrintStatus();
		}

		g_FPS = gFrameSkipper.GetFrameCount();
		g_LPS = gFrameSkipper.GetLoopCounter();

		gFrameSkipper.Refresh();
	}

	ULONGLONG curTime = GetTickCount64();

	for (auto iter = g_users.begin(); iter != g_users.end();)
	{
		User* user = iter->second;
		Session* session = FindSession(user->userNo);

		iter++;
		//if (user->hp <= 0)
		//{
		//	//DisconnectProc(session->GetSessionNo());
		//	continue;
		//}

		// Timeout Func
		/*if (curTime - session->GetLastRecvTime() > 60000)
		{
			g_Logger._Log(dfLOG_LEVEL_NOTICE, L"[UserNo: %d] Time Out!\n",
				user->userNo);
			DisconnectProc(session->GetSessionNo());
			continue;
		}*/

		switch (user->action)
		{
		case dfACTION_MOVE_LL:
			if (UserMoveCheck(user->x - dfSPEED_PLAYER_X, user->y))
			{
				user->x -= dfSPEED_PLAYER_X;
			}
			break;
		case dfACTION_MOVE_LU:
			if (UserMoveCheck(user->x - dfSPEED_PLAYER_X, user->y - dfSPEED_PLAYER_Y))
			{
				user->x -= dfSPEED_PLAYER_X;
				user->y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_UU:
			if (UserMoveCheck(user->x, user->y - dfSPEED_PLAYER_Y))
			{
				user->y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_RU:
			if (UserMoveCheck(user->x + dfSPEED_PLAYER_X, user->y - dfSPEED_PLAYER_Y))
			{
				user->x += dfSPEED_PLAYER_X;
				user->y -= dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_RR:
			if (UserMoveCheck(user->x + dfSPEED_PLAYER_X, user->y))
			{
				user->x += dfSPEED_PLAYER_X;
			}
			break;
		case dfACTION_MOVE_RD:
			if (UserMoveCheck(user->x + dfSPEED_PLAYER_X, user->y + dfSPEED_PLAYER_Y))
			{
				user->x += dfSPEED_PLAYER_X;
				user->y += dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_DD:
			if (UserMoveCheck(user->x, user->y + dfSPEED_PLAYER_Y))
			{
				user->y += dfSPEED_PLAYER_Y;
			}
			break;
		case dfACTION_MOVE_LD:
			if (UserMoveCheck(user->x - dfSPEED_PLAYER_X, user->y + dfSPEED_PLAYER_Y))
			{
				user->x -= dfSPEED_PLAYER_X;
				user->y += dfSPEED_PLAYER_Y;
			}
			break;
		default:
			continue;
			break;
		}

		if (Sector_UpdateUser(user))
		{
			UserSectorUpdatePacket(user);
		}
	}
}

void ServerControl()
{
	if (_kbhit())
	{
		WCHAR key = _getwch();
		rewind(stdin);

		if (key == L'Q')
		{
			g_Shutdown = true;
		}

		if (key == L'I')
		{
			wprintf(L"==========================\n");
			wprintf_s(L"Program Exit[Q]\nDebug Mode[D]\nError Mode[E]\nProfile Write[H]\nMonitoring Mode[M]\n");
			wprintf(L"==========================\n");
		}

		if (key == L'D')
		{
			g_Logger.setLogLevel(dfLOG_LEVEL_DEBUG);
			wprintf_s(L"Set Debug Mode\n");

		}

		if (key == L'E')
		{
			g_Logger.setLogLevel(dfLOG_LEVEL_ERROR);
			wprintf_s(L"Set Error Mode\n");
		}

		if (key == L'M')
		{
			if (g_bMonitoring)
			{
				g_bMonitoring = false;
				wprintf_s(L"Unset Monitoring Mode\n");
			}
			else
			{
				g_bMonitoring = true;
				wprintf_s(L"Set Monitoring Mode\n");
			}
			
		}

		if (key == L'H')
		{
			ProfileDataOutText(L"Profile");
			ProfileReset();
		}		
	}
}

void Monitor()
{
	wprintf_s(L"[User: %u] [Frame: %d]\n============================\n", g_ConnectCount - g_DisconnectCount, g_FPS);
	wprintf_s(L"[Loop/sec: %d]\n============================\n", g_LPS);
	ProfilePrint();
	wprintf_s(L"============================\n");
	wprintf_s(L"[SendTPS: %u]\n[RecvTPS: %u]\n", g_sendTPS, g_recvTPS);
	wprintf_s(L"============================\n");

	ProfileReset();

	g_sendTPS = 0;
	g_recvTPS = 0;

	g_bMonitorTimer = false;
}
