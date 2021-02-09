#ifndef GAMEOBJECT
#define GAMEOBJECT

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ENEMY (100)
#define MAX_BULLET (500)
#define MAX_ENEMY_NUMBER (5)

enum TYPE
{
	TYPE_PLAYER,
	TYPE_WEAK_ENEMY,
	TYPE_STRONG_ENEMY,
	TYPE_BOSS
};

enum DIRECTION
{
	DIRECTION_IDLE,
	DIRECTION_UP = 1,
	DIRECTION_RIGHT = 2,
	DIRECTION_DOWN = 4,
	DIRECTION_LEFT = 8
};

struct Bullet
{
	bool bIsAlive;
	BYTE dir;
	char x;
	char y;
	BYTE damage;
	char image;
	bool bIsEnemy;
};

struct Unit
{
	BYTE HP;
	char x;
	char y;
	TYPE type;
};

struct Unit_Stat
{
	BYTE HP;
	BYTE damage;
	char image;
};

static const char unit_list_file[32] = "unit_stat.txt";
extern Unit g_player;
extern Unit_Stat g_unit_stat[MAX_ENEMY_NUMBER];
extern Unit g_enemy[MAX_ENEMY];
extern Bullet g_bullet[MAX_BULLET];

bool initialize_global_unit_data();
bool destroy_global_unit_data();
void make_setting_files();

#endif