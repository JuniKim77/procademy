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

using namespace std;

extern bool g_Shutdown;
extern unordered_map<DWORD, User*> g_users;
extern FrameSkip gFrameSkipper;
extern CLogger g_Logger;

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
		int frameCount = gFrameSkipper.GetFrameCount();

		if (frameCount >= 52 || frameCount <= 48)
		{
			g_Logger._Log(dfLOG_LEVEL_DEBUG, L"[Frame Count: %d][Loop Count: %d]\n", 
				gFrameSkipper.GetFrameCount(), gFrameSkipper.GetLoopCounter());
		}

		gFrameSkipper.Refresh();
	}

	ULONGLONG curTime = GetTickCount64();

	for (auto iter = g_users.begin(); iter != g_users.end();)
	{
		User* user = iter->second;
		Session* session = FindSession(user->userNo);

		iter++;
		// HP가 0이하면 종료 처리..
		if (user->hp <= 0)
		{
			//DisconnectProc(session->GetSessionNo());
			continue;
		}

		// 수신 종료 처리
		if (curTime - session->GetLastRecvTime() > 60000)
		{
			g_Logger._Log(dfLOG_LEVEL_NOTICE, L"[UserNo: %d] Time Out!\n",
				user->userNo);
			DisconnectProc(session->GetSessionNo());
			continue;
		}

		switch (user->moveDirection)
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

		// Q키 : 프로그램 종료
		if (key == L'Q')
		{
			g_Shutdown = true;
		}
		// I키 : 키 정보
		if (key == L'I')
		{
			wprintf_s(L"Program Exit : Shift + Q\nDebug Mode : Shift + D\nError Mode : Shift + E\n");
		}
		// D키 : 디버그 모드 전환
		if (key == L'D')
		{
			g_Logger.setLogLevel(dfLOG_LEVEL_DEBUG);
		}
		// E키 : 에러 모드 전환
		if (key == L'E')
		{
			g_Logger.setLogLevel(dfLOG_LEVEL_ERROR);
		}
	}
}

void Monitor()
{
}
