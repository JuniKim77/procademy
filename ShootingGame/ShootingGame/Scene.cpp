#include "Scene.h"

int g_scene;
int g_stage;
bool gb_user_bullet;
static char load_files[FILE_MAX_NUM][FILE_NAME_SIZE];

bool initialize_global_scene_data()
{
	FILE* fin;
	char* file_memory;

	fopen_s(&fin, file_list, "r");
	if (fin == nullptr)
	{
		printf("%s is not found.\n", file_list);
		return false;
	}

	fseek(fin, 0, SEEK_END);
	int file_size = ftell(fin) + 1;

	file_memory = (char*)malloc(file_size);
	if (file_memory == nullptr)
	{
		printf("Failed file_memory malloc.\n");
		return false;
	}
	
	fseek(fin, 0, SEEK_SET);
	fread_s(file_memory, file_size, file_size, 1, fin);
	char* p_file_memeory = file_memory;

	char* buffer = nullptr;

	buffer = strtok_s(buffer, "\n", &p_file_memeory);
	s_file_num = atoi(buffer);

	for (int i = 0; i < s_file_num; ++i)
	{
		buffer = strtok_s(nullptr, "\n", &p_file_memeory);

		memcpy(load_files[i], buffer, FILE_NAME_SIZE);
	}

	free(file_memory);
	fclose(fin);

	g_scene = SCENE_LOADING;
	g_stage = 0;
	gb_user_bullet = false;

	return true;
}

bool load_entry_scene()
{
	static bool message_on = true;
	static int prev_time = timeGetTime();

	int cur_time = timeGetTime();

	if (cur_time - prev_time >= 1000)
	{
		if (message_on)
		{
			char start_str[] = "Press enter to start game ...";
			sprintf_s(szScreenBufferForCopy[18] + 25, dfSCREEN_WIDTH - 25, start_str);
			szScreenBufferForCopy[18][strlen(start_str) + 25] = ' ';
			message_on = false;
		}
		else
		{
			memset(szScreenBufferForCopy[18], ' ', dfSCREEN_WIDTH);
			szScreenBufferForCopy[18][dfSCREEN_WIDTH - 1] = '\0';
			message_on = true;
		}
		prev_time = cur_time;
	}

	memcpy_s(szScreenBuffer, sizeof(szScreenBuffer), szScreenBufferForCopy, sizeof(szScreenBufferForCopy));

	return true;
}

bool get_key_change_entry()
{
	if (GetAsyncKeyState(VK_RETURN) & 0x8001)
	{
		g_scene = SCENE_LOADING;
		g_stage++;
		return true;
	}

	return false;
}

bool get_key_change_play()
{
	DWORD cur_time = timeGetTime();

	if (GetAsyncKeyState(VK_SPACE) & 0x8001)
	{
		if (!gb_user_bullet)
		{
			for (int i = 0; i < MAX_BULLET; ++i)
			{
				if (!g_bullet[i].bIsAlive)
				{
					g_bullet[i].dir = DIRECTION_UP;
					g_bullet[i].x = g_player.x;
					g_bullet[i].y = g_player.y - 1;
					g_bullet[i].bIsAlive = true;
					g_bullet[i].speed = 2;
					g_bullet[i].damage = 1;
					g_bullet[i].bIsEnemy = false;
					g_bullet[i].prev_time = cur_time;

					gb_user_bullet = true;
					break;
				}
			}
		}
	}

	if (GetAsyncKeyState(VK_UP) & 0x8001)
	{
		g_player.y = max(0, g_player.y - 1);
		gb_user_bullet = false;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8001)
	{
		g_player.y = min(dfSCREEN_HEIGHT - 1, g_player.y + 1);
		gb_user_bullet = false;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
	{
		g_player.x = min(dfSCREEN_WIDTH - 2, g_player.x + 1);
		gb_user_bullet = false;
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8001)
	{
		g_player.x = max(0, g_player.x - 1);
		gb_user_bullet = false;
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8001)
	{

		return true;
	}

	return false;
}

bool load_play_scene()
{
	if (g_player.HP > 0)
	{
		szScreenBuffer[g_player.y][g_player.x] = g_unit_stat[TYPE_PLAYER].image;
	}

	for (int i = 0; i < MAX_ENEMY; ++i)
	{
		Unit enemy = g_enemy[i];

		if (enemy.HP > 0)
		{
			szScreenBuffer[enemy.y][enemy.x] = g_unit_stat[enemy.type].image;
		}
	}

	for (int i = 0; i < MAX_BULLET; ++i)
	{
		Bullet bullet = g_bullet[i];

		if (bullet.bIsAlive)
		{
			szScreenBuffer[bullet.y][bullet.x] = '*';
		}
	}

	return true;
}

bool process_play_logic()
{
	static DWORD prev_time = timeGetTime();
	static int movement = 0;

	// 적 유닛 움직임 제어

	BYTE unit_move_tbl[3][8] = {
		{DIRECTION_UP, DIRECTION_DOWN, DIRECTION_RIGHT, DIRECTION_LEFT, DIRECTION_DOWN, DIRECTION_UP, DIRECTION_LEFT, DIRECTION_RIGHT},
		{DIRECTION_LEFT, DIRECTION_LEFT, DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_LEFT, DIRECTION_LEFT},
		{DIRECTION_UP | DIRECTION_LEFT, DIRECTION_RIGHT, DIRECTION_RIGHT, DIRECTION_DOWN, DIRECTION_DOWN, DIRECTION_LEFT, DIRECTION_LEFT, DIRECTION_UP | DIRECTION_RIGHT}
	};

	DWORD cur_time = timeGetTime();

	if (cur_time - prev_time > 1000)
	{
		prev_time = cur_time;
		for (int i = 0; i < MAX_ENEMY; ++i)
		{
			if (g_enemy[i].HP == 0)
				continue;

			BYTE dir = unit_move_tbl[g_enemy[i].type - 1][movement];

			if (dir & DIRECTION_UP)
			{
				g_enemy[i].y -= 1;
			}
			if (dir & DIRECTION_RIGHT)
			{
				g_enemy[i].x += 1;
			}
			if (dir & DIRECTION_DOWN)
			{
				g_enemy[i].y += 1;
			}
			if (dir & DIRECTION_LEFT)
			{
				g_enemy[i].x -= 1;
			}
		}
		
		movement = (movement + 1) % 8;
	}

	// 총알 제어

	for (int i = 0; i < MAX_BULLET; ++i)
	{
		if (!g_bullet[i].bIsAlive) 
		{
			continue;
		}

		BYTE dir = g_bullet[i].dir;

		// 총알 움직임
		if (cur_time - g_bullet[i].prev_time > 1000 / g_bullet[i].speed)
		{
			if (dir & DIRECTION_UP)
			{
				g_bullet[i].y -= 1;
				if (g_bullet[i].y < 0) 
				{
					g_bullet[i].bIsAlive = false;
				}
			}
			if (dir & DIRECTION_RIGHT)
			{
				g_bullet[i].x += 1;
			}
			if (dir & DIRECTION_DOWN)
			{
				g_bullet[i].y += 1;
				if (g_bullet[i].y >= dfSCREEN_HEIGHT)
				{
					g_bullet[i].bIsAlive = false;
				}
			}
			if (dir & DIRECTION_LEFT)
			{
				g_bullet[i].x -= 1;
			}
			if (g_bullet[i].bIsEnemy == false)
			{
				gb_user_bullet = false;
			}

			// 총알 충돌 처리
			Bullet cur_bullet = g_bullet[i];

			if (cur_bullet.bIsEnemy)
			{
				if (g_player.x == cur_bullet.x && g_player.y == cur_bullet.y)
				{
					g_player.HP--;
					g_bullet[i].bIsAlive = false;
					g_bullet[i].x = -1;
					g_bullet[i].y = -1;

					if (g_player.HP == 0)
					{
						return true;
					}

					continue;
				}
			}

			if (cur_bullet.bIsEnemy == false)
			{
				for (int j = 0; j < MAX_ENEMY; ++j)
				{
					if (g_enemy[j].HP > 0)
					{
						if (g_enemy[j].x == cur_bullet.x && g_enemy[j].y == cur_bullet.y)
						{
							g_enemy[j].HP = max(0, g_enemy[j].HP - cur_bullet.damage);
							g_bullet[i].bIsAlive = false;
							g_bullet[i].x = -1;
							g_bullet[i].y = -1;

							continue;
						}
					}
				}
			}

			g_bullet[i].prev_time = cur_time;
		}
	}

	return false;
}

bool load_end_scene()
{
	return false;
}

bool load_loading_scene()
{
	FILE* fin;
	fopen_s(&fin, load_files[g_stage], "r");
	if (fin == nullptr)
	{
		printf("Failed open file, %s.\n", load_files[g_stage]);
		return false;
	}

	fseek(fin, 0, SEEK_END);
	int file_size = ftell(fin) + 1;
	char* file_memory = (char*)malloc(file_size);

	if (file_memory == nullptr)
	{
		printf("Failed buffer malloc.\n");
		return false;
	}
	fseek(fin, 0, SEEK_SET);
	fread_s(file_memory, file_size, file_size, 1, fin);

	char* buffer = nullptr;
	char* p_file_memeory = file_memory;

	buffer = strtok_s(buffer, "\n", &p_file_memeory);
	buffer = strtok_s(nullptr, "\n", &p_file_memeory);
	buffer = strtok_s(nullptr, "\n", &p_file_memeory);

	if (g_stage == 0) // Entry Scene
	{
		memset(szScreenBufferForCopy, ' ', sizeof(szScreenBufferForCopy));
		for (int i = 0; i < dfSCREEN_HEIGHT; ++i)
		{
			szScreenBufferForCopy[i][dfSCREEN_WIDTH - 1] = '\0';
		}

		while (1)
		{
			buffer = strtok_s(nullptr, "\n", &p_file_memeory);
			if (buffer == nullptr)
			{
				break;
			}

			int x = atoi(strtok_s(buffer, " ", &buffer));
			int y = atoi(strtok_s(nullptr, " ", &buffer));

			szScreenBufferForCopy[y][x] = *strtok_s(nullptr, " ", &buffer);
		}

		g_scene = SCENE_ENTRY;
	}
	else if (g_stage == s_file_num - 1) // End Scene
	{
		g_scene = SCENE_END;
	}
	else // Play Scene
	{
		while (1)
		{
			buffer = strtok_s(nullptr, "\n", &p_file_memeory);
			if (buffer == nullptr)
			{
				break;
			}

			int x = atoi(strtok_s(buffer, " ", &buffer));
			int y = atoi(strtok_s(nullptr, " ", &buffer));
			int type = atoi(strtok_s(nullptr, " ", &buffer));

			for (int i = 0; i < MAX_ENEMY; ++i)
			{
				if (g_enemy[i].HP == 0)
				{
					g_enemy[i].HP = g_unit_stat[type].HP;
					g_enemy[i].x = x;
					g_enemy[i].y = y;
					g_enemy[i].type = (TYPE)type;
					break;
				}
			}
		}

		g_scene = SCENE_PLAY;
	}

	fclose(fin);
	free(file_memory);

	return true;
}

void destroy_global_scene_data()
{

}