#ifndef SCENE
#define SCENE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>
#include "GameObject.h"

#define FILE_MAX_NUM (32)
#define FILE_NAME_SIZE (64)
#define dfSCREEN_WIDTH		81		// �ܼ� ���� 80ĭ + NULL
#define dfSCREEN_HEIGHT		24		// �ܼ� ���� 24ĭ

enum Scene
{
	SCENE_ENTRY,
	SCENE_PLAY,
	SCENE_END,
	SCENE_LOADING
};

static const char file_list[32] = "file_list.txt";
static int s_file_num;
extern int g_scene;
extern int g_stage;
extern bool gb_user_bullet;
extern char szScreenBuffer[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];
extern char szScreenBufferForCopy[dfSCREEN_HEIGHT][dfSCREEN_WIDTH];

// ���� ���� ����
bool initialize_global_scene_data();
void destroy_global_scene_data();

// ��Ʈ��
bool load_entry_scene();
bool get_key_change_entry();

// ���� �÷���
bool load_play_scene();
bool get_key_change_play();
bool process_play_logic();

// ����
bool load_end_scene();
bool load_loading_scene();


#endif