#pragma once

#include <cpp_redis/cpp_redis>

#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")

namespace procademy
{
	class CRedis_TLS
	{
	public:
		static void					InitRedis_TLS(const WCHAR* redisIP, int redisPort, int num);
		static void					DestroyRedis_TLS();
		static void					SetRedis(const char* key, int time, std::string& value);

	private:
		static cpp_redis::client*	GetRedis();

	private:
		static cpp_redis::client*	s_RedisArray;
		static DWORD				s_RedisTls;
		static LONG					s_RedisIdx;
		static LONG					s_RedisNum;
		static SRWLOCK				s_lock;
	};
}

