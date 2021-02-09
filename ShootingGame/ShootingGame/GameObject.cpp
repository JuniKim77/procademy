#include "GameObject.h"

Unit g_player;
Unit_Stat g_unit_stat[MAX_ENEMY_NUMBER];
Unit g_enemy[MAX_ENEMY];
Bullet g_bullet[MAX_BULLET];

bool initialize_global_unit_data()
{
	// Stat Data 처리
	FILE* fin;
	char* file_memory;

	fopen_s(&fin, unit_list_file, "r");

	fseek(fin, 0, SEEK_END);
	int file_size = ftell(fin) + 1;

	file_memory = (char*)malloc(file_size);

	fseek(fin, 0, SEEK_SET);
	fread_s(file_memory, file_size, file_size, 1, fin);
	char* p_file_memeory = file_memory;

	char* buffer = nullptr;

	buffer = strtok_s(buffer, "\n", &p_file_memeory);
	buffer = strtok_s(nullptr, "\n", &p_file_memeory);
	buffer = strtok_s(nullptr, "\n", &p_file_memeory);

	while (1)
	{
		buffer = strtok_s(nullptr, "\n", &p_file_memeory);

		if (buffer == nullptr)
		{
			break;
		}

		char* data = nullptr;
		data = strtok_s(data, " ", &buffer);
		data = strtok_s(nullptr, " ", &buffer);

		int type = atoi(data);
		
		data = strtok_s(nullptr, " ", &buffer);
		g_unit_stat[type].HP = atoi(data);

		data = strtok_s(nullptr, " ", &buffer);
		g_unit_stat[type].damage = atoi(data);

		data = strtok_s(nullptr, " ", &buffer);
		g_unit_stat[type].image = *data;
	}

	free(file_memory);
	fclose(fin);

	// 플레이어 데이터 초기화
	g_player.HP = g_unit_stat[TYPE_PLAYER].HP;
	g_player.x = 40;
	g_player.y = 21;
	g_player.type = TYPE_PLAYER;

	memset(g_enemy, 0, sizeof(g_enemy));
	memset(g_bullet, 0, sizeof(g_bullet));

	return false;
}

bool destroy_global_unit_data()
{
	return false;
}

void make_setting_files()
{
	FILE* fin;
	fopen_s(&fin, "scene_stage1.txt", "w");

	const char init[] = "Enter unit initial positions following below order\nx y type\n=====\n";

	if (fwrite(init, sizeof(init) - 1, 1, fin) != 1)
	{
		printf("fwrite error\n");
		return;
	}

	const int col_num = 5;
	const int row_num = 3;
	const int row_begin = 1;
	char buffer[1024];
	int count = 0;

	for (int i = 0; i < row_num; ++i)
	{
		char temp[16];

		sprintf_s(temp, "%d %d %d\n", 40, row_begin + 2 * i, 1);
		memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
		for (int j = 1; j <= col_num; ++j)
		{
			sprintf_s(temp, "%d %d %d\n", 40 + 5 * j, row_begin + 2 * i, 1);
			memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
			sprintf_s(temp, "%d %d %d\n", 40 - 5 * j, row_begin + 2 * i, 1);
			memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
		}
	}
	if (fwrite(buffer, strlen(buffer) - 1, 1, fin) != 1)
	{
		printf("fwrite error\n");
		return;
	}

	fclose(fin);

	fopen_s(&fin, "scene_stage2.txt", "w");

	if (fwrite(init, sizeof(init) - 1, 1, fin) != 1)
	{
		printf("fwrite error\n");
		return;
	}

	memset(buffer, 0, sizeof(buffer));
	count = 0;

	for (int i = 0; i < row_num; ++i)
	{
		char temp[16];

		sprintf_s(temp, "%d %d %d\n", 40, row_begin + 2 * i, 2);
		memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
		for (int j = 1; j <= col_num; ++j)
		{
			sprintf_s(temp, "%d %d %d\n", 40 + 5 * j, row_begin + 2 * i, 2);
			memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
			sprintf_s(temp, "%d %d %d\n", 40 - 5 * j, row_begin + 2 * i, 2);
			memcpy_s(buffer + count++ * strlen(temp), sizeof(buffer), temp, sizeof(temp));
		}
	}

	if (fwrite(buffer, strlen(buffer) - 1, 1, fin) != 1)
	{
		printf("fwrite error\n");
		return;
	}

	fclose(fin);
}
