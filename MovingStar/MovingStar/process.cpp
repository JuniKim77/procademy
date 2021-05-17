#include "process.h"
#include "star.h"
#include <WinSock2.h>
#include <stdio.h>
#include <wchar.h>
#include "message.h"
#include "RingBuffer.h"

#define STAR_SIZE (100)

star stars[STAR_SIZE];
int num_stars = 0;
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

void NetworkProcess(SOCKET server, RingBuffer* ringBuffer)
{
	FD_SET rset;
	FD_SET wset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_SET(server, &rset);

	if (stars[0].x != stars[0].oldX || stars[0].y != stars[0].oldY) {
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
		receiveMessage(server, &rset, ringBuffer);

		receiveMessageProc(ringBuffer);

		sendMessageProc(server, &wset);
	}
	else if (retval == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		wprintf_s(L"Error code : %d\n", err);
		exit(1);
	}
}

void ScreenInitial()
{
	CONSOLE_CURSOR_INFO stConsoleCursor;

	//-------------------------------------------------------------
	// ȭ���� Ŀ���� �Ⱥ��̰Բ� �����Ѵ�.
	//-------------------------------------------------------------
	stConsoleCursor.bVisible = FALSE;
	stConsoleCursor.dwSize = 1;			// Ŀ�� ũ��.
											// �̻��ϰԵ� 0 �̸� ���´�. 1���ϸ� �ȳ��´�.

	//-------------------------------------------------------------
	// �ܼ�ȭ�� (���ٴٵ� �ƿ�ǲ) �ڵ��� ���Ѵ�.
	//-------------------------------------------------------------
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorInfo(hConsole, &stConsoleCursor);
}

void receiveMessage(SOCKET server, FD_SET* rset, RingBuffer* ringBuffer)
{
	if (FD_ISSET(server, rset))
	{
		int retval = recv(server, ringBuffer->GetRearBufferPtr(), ringBuffer->GetFreeSize(), 0);

		if (retval == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err != WSAEWOULDBLOCK) {
				system("cls");
				wprintf_s(L"Recv Error code : %d\n", err);
				exit(1);
			}
		}

		ringBuffer->MoveRear(retval);
	}
}

void MoveCursor(int posX, int posY)
{
	COORD coord;
	coord.X = posX;
	coord.Y = posY;

	SetConsoleCursorPosition(hConsole, coord);
}

void receiveMessageProc(RingBuffer* ringBuffer)
{
	char rPacket[16];

	while (1)
	{
		if (ringBuffer->GetUseSize() < 16)
		{
			break;
		}

		ringBuffer->Dequeue(rPacket, 16);

		int* type = (int*)rPacket;

		switch (*type)
		{
		case 0: // ID �Ҵ�
			stars[0].ID = ((ID_ALLOCATE*)rPacket)->ID;
			break;
		case 1: // �� ����
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
		case 2: // �� ����
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
		case 3: // �� �̵�
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
	}
}

void sendMessageProc(SOCKET server, FD_SET* wset)
{
	if (FD_ISSET(server, wset))
	{
		MOVE_STAR message = MOVE_STAR(stars[0].ID, stars[0].x, stars[0].y);
		char* pMessage = (char*)&message;

		if (send(server, pMessage, sizeof(MOVE_STAR), 0) == SOCKET_ERROR)
		{
			system("cls");
			wprintf_s(L"�޼��� �۽� ���� Code : %d\n", WSAGetLastError());
			exit(1);
		}
	}
}
