#pragma once
#include <WinSock2.h>

void Render();
void KeyProcess();
void NetworkProcess(SOCKET server);
void ScreenClear();
void MoveCursor(int posX, int posY);
void ScreenInitial();