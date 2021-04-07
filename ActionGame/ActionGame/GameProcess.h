#pragma once
#include <Windows.h>

class ScreenDib;
class SpriteDib;

enum Process
{
	PROCESS_TITLE,
	PROCESS_GAME
};

extern ScreenDib gScreenDib;
extern Process gGameState;
extern HWND gMainWindow;
extern bool gbActiveApp;
extern SpriteDib gSpriteDib;

void InitializeGame();
void ContentLoad();
void RunGame();
void UpdateGame();
void KeyProcess();
void Update();
void Render();
void SortYaxis();