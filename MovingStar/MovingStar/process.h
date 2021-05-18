#pragma once
#include <WinSock2.h>

void Render();
void KeyProcess();
void NetworkProcess(SOCKET server);
void MoveCursor(int posX, int posY);
void ScreenInitial();
void receiveMessageProc(SOCKET server, FD_SET* rset);
void sendMessageProc(SOCKET server, FD_SET* wset);