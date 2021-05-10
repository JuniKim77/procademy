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
	friend void KeyProcess();
	friend void NetworkProcess(SOCKET server);
public:
	star();
	void setID(int id) { ID = id; }
	bool isEqual(int id) { return ID == id; }
	void move(Dir dir);
	void save();

private:
	int x;
	int oldX;
	int y;
	int oldY;
	int ID;
};