#include "process.h"
#include "star.h"
#include <WinSock2.h>
#include <stdio.h>
#include <wchar.h>
#include "message.h"

#define STAR_SIZE (100)

star stars[STAR_SIZE];
int num_stars = 0;
WCHAR szScreenBuffer[WINDOW_HEIGHT][WINDOW_WIDTH + 1];
HANDLE hConsole;

void Render()
{
	MoveCursor(0, 0);
	wprintf_s(L"Connect Client : %d", num_stars);

	for (int i = 0; i < num_stars; ++i)
	{
		MoveCursor(stars[i].oldX, stars[i].oldY);
		fputwc(L' ', stdout);
		MoveCursor(stars[i].x, stars[i].y);
		fputwc(L'*', stdout);
	}
}

void KeyProcess()
{
	stars[0].save();

	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		stars[0].move(Dir::DIR_UP);
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		stars[0].move(Dir::DIR_DOWN);
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		stars[0].move(Dir::DIR_LEFT);
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		stars[0].move(Dir::DIR_RIGHT);
	}
}

void NetworkProcess(SOCKET server)
{
	FD_SET rset;
	FD_SET wset;
	char wPacket[16] = { 0, };
	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(server, &rset);

	if (stars[0].x != stars[0].oldX || stars[0].y != stars[0].oldY) {
		// send Message
		MOVE_STAR* pMessage = (MOVE_STAR*)wPacket;
		*pMessage = MOVE_STAR(stars[0].ID, stars[0].x, stars[0].y);
		FD_SET(server, &wset);
	}

	//select
	timeval tval;
	tval.tv_sec = 0;
	tval.tv_usec = 0;
	int count = 0;
	
	int retval = select(0, &rset, &wset, NULL, &tval);

	if (retval > 0)
	{
		if (FD_ISSET(server, &rset))
		{
			char rPacket[16];

			retval = recv(server, rPacket, sizeof(rPacket), 0);

			if (retval == SOCKET_ERROR)
			{
				int err = WSAGetLastError();

				if (err != WSAEWOULDBLOCK) {
					system("cls");
					wprintf_s(L"Error code : %d\n", err);
					exit(1);
				}
			}

			while (retval > 0)
			{
				int* type = (int*)rPacket;
				count++;

				switch (*type)
				{
				case 0: // ID 할당
					stars[0].ID = ((ID_ALLOCATE*)rPacket)->ID;
					break;
				case 1: // 별 생성
					if (((CREATE_STAR*)rPacket)->ID == stars[0].ID)
					{
						stars[0].x = ((CREATE_STAR*)rPacket)->x;
						stars[0].y = ((CREATE_STAR*)rPacket)->y;
					}
					else
					{
						if (num_stars >= STAR_SIZE) {
							continue;
						}
						stars[num_stars].ID = ((CREATE_STAR*)rPacket)->ID;
						stars[num_stars].x = ((CREATE_STAR*)rPacket)->x;
						stars[num_stars].oldX = ((CREATE_STAR*)rPacket)->x;
						stars[num_stars].y = ((CREATE_STAR*)rPacket)->y;
						stars[num_stars].oldY = ((CREATE_STAR*)rPacket)->y;
					}
					num_stars++;
					break;
				case 2: // 별 삭제
					for (int i = 0; i < num_stars; ++i)
					{
						if (stars[i].ID == ((DESTROY_STAR*)rPacket)->ID)
						{
							MoveCursor(stars[i].x, stars[i].y);
							fputwc(L' ', stdout);
							stars[i] = stars[num_stars - 1];
							num_stars--;
							break;
						}
					}
					break;
				case 3: // 별 이동
					for (int i = 0; i < num_stars; ++i)
					{
						if (stars[i].ID == ((MOVE_STAR*)rPacket)->ID)
						{
							stars[i].oldX = stars[i].x;
							stars[i].oldY = stars[i].y;
							stars[i].x = ((MOVE_STAR*)rPacket)->x;
							stars[i].y = ((MOVE_STAR*)rPacket)->y;
							MoveCursor(stars[i].oldX, stars[i].oldY);
							fputwc(L' ', stdout);
							break;
						}
					}
					break;
				default:
					break;
				}

				retval = recv(server, rPacket, sizeof(rPacket), 0);
			}
		}

		if (FD_ISSET(server, &wset))
		{
			if (send(server, wPacket, sizeof(wPacket), 0) == SOCKET_ERROR)
			{
				wprintf_s(L"메세지 송신 에러 Code : %d\n", WSAGetLastError());
				exit(1);
			}
		}
	}
	else if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf_s(L"Error code : %d\n", err);
		exit(1);
	}

	MoveCursor(0, 1);
	wprintf_s(L"Packet: %d ", count);
	MoveCursor(0, 2);
	wprintf_s(L"ID: %d", stars[0].ID);
}

void ScreenInitial()
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	//-------------------------------------------------------------
	// 화면의 커서를 안보이게끔 설정한다.
	//-------------------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;			// 커서 크기.
											// 이상하게도 0 이면 나온다. 1로하면 안나온다.

	//-------------------------------------------------------------
	// 콘솔화면 (스텐다드 아웃풋) 핸들을 구한다.
	//-------------------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

void MoveCursor(int posX, int posY)
{
	COORD coord;
	coord.X = posX;
	coord.Y = posY;

	SetConsoleCursorPosition(hConsole, coord);
}
