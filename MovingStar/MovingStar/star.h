#pragma once
#include <WinSock2.h>
#define WINDOW_WIDTH (80)
#define WINDOW_HEIGHT (23)


enum class Dir
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

class star
{
	friend void Render();
	friend void NetworkProcess(SOCKET server);
public:
	star();
	void setID(int id) { ID = id; }
	bool isEqual(int id) { return ID == id; }
	void move(Dir dir);
	bool isMoved() { return mbMoved; }
	void unsetMoved() { mbMoved = false; }

private:
	int x;
	int y;
	int ID;
	bool mbMoved;
};