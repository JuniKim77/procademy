#pragma once
#include <WinSock2.h>

class RingBuffer;

void Render();
void KeyProcess();
void NetworkProcess(SOCKET server, RingBuffer* ringBuffer);
void MoveCursor(int posX, int posY);
void ScreenInitial();
void receiveMessage(SOCKET server, FD_SET* rset, RingBuffer* ringBuffer);
void receiveMessageProc(RingBuffer* ringBuffer);
void sendMessageProc(SOCKET server, FD_SET* wset);