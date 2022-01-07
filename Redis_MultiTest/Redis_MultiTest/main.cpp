#include <cpp_redis/cpp_redis>
#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")
#pragma comment (lib, "ws2_32.lib")
#include "TextParser.h"
#include "CProfiler.h"

void LoadFiles(const WCHAR* file);
unsigned int WINAPI WorkerThread(LPVOID arg);

int g_thread_num;
int g_chunk_size;
int g_try_num;

cpp_redis::client* g_redis;

char numbers[100000][8];

int main()
{
	CProfiler::InitProfiler(10);

	LoadFiles(L"Setting.ini");

	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);

	HANDLE* hThreads = new HANDLE[g_thread_num];

	for (int i = 0; i < g_thread_num * g_chunk_size; ++i)
	{
		_itoa_s(i, numbers[i], 8, 10);
	}

	cpp_redis::client client;

	client.connect();

	g_redis = &client;

	int args[10];

	for (int i = 0; i < 10; ++i)
	{
		args[i] = i;
	}

	for (int i = 0; i < g_thread_num; ++i)
	{
		hThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, &args[i], 0, nullptr);
	}

	DWORD ret = WaitForMultipleObjects(g_thread_num, hThreads, true, INFINITE);

	switch (ret)
	{
	case WAIT_FAILED:
		wprintf_s(L"Redis Multi Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		wprintf_s(L"Redis Multi TimeOut Error\n");
		break;
	case WAIT_OBJECT_0:
		wprintf_s(L"Redis Multi End\n");
		break;
	default:
		break;
	}

	for (int j = 0; j < g_try_num; ++j)
	{
		CProfiler::Begin(L"Redis_Single");
		{
			for (int i = 0; i < g_thread_num * g_chunk_size; ++i)
			{
				g_redis->set(numbers[i], "1");

				g_redis->get(numbers[i], [](cpp_redis::reply& reply) {
					if (reply.as_string() != "1")
					{
						printf_s("Not Mached %s\n", reply.as_string().c_str());
					}
					});

				g_redis->sync_commit();
			}
		}
		CProfiler::End(L"Redis_Single");
	}

	CProfiler::Print();

	client.disconnect();

	return 0;
}

void LoadFiles(const WCHAR* file)
{
	TextParser tp;

	tp.LoadFile(file);

	tp.GetValue(L"THREAD_NUM", &g_thread_num);
	tp.GetValue(L"CHUNK_SIZE", &g_chunk_size);
	tp.GetValue(L"TRY_NUM", &g_try_num);
}

unsigned int __stdcall WorkerThread(LPVOID arg)
{
	int num = *(int*)arg;

	for (int j = 0; j < g_try_num; ++j)
	{
		CProfiler::Begin(L"Redis_Multi");
		{
			for (int i = num * g_chunk_size; i < (num + 1) * g_chunk_size; ++i)
			{
				g_redis->set(numbers[i], "1");

				g_redis->get(numbers[i], [](cpp_redis::reply& reply) {
					if (reply.as_string() != "1")
					{
						printf_s("Not Mached %s\n", reply.as_string().c_str());
					}
					});

				g_redis->sync_commit();
			}
		}
		CProfiler::End(L"Redis_Multi");
	}

	return 0;
}
