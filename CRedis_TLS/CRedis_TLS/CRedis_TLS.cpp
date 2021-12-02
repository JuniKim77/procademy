#include "CRedis_TLS.h"

namespace procademy
{
	cpp_redis::client*	CRedis_TLS::s_RedisArray;
	DWORD				CRedis_TLS::s_RedisTls;
	LONG				CRedis_TLS::s_RedisIdx;
	LONG				CRedis_TLS::s_RedisNum;
	SRWLOCK				CRedis_TLS::s_lock;
}

void procademy::CRedis_TLS::InitRedis_TLS(const WCHAR* redisIP, int redisPort, int num)
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(version, &data);

	char IP[64];

	WideCharToMultiByte(CP_ACP, 0, redisIP, -1, IP, sizeof(IP), nullptr, nullptr);

	s_RedisTls = TlsAlloc();
	InitializeSRWLock(&s_lock);
	s_RedisArray = new cpp_redis::client[num];
	s_RedisNum = num;

	for (int i = 0; i < num; ++i)
	{
		s_RedisArray[i].connect(IP, redisPort);
	}
}

void procademy::CRedis_TLS::DestroyRedis_TLS()
{
	for (int i = 0; i < s_RedisNum; ++i)
	{
		s_RedisArray[i].disconnect();
	}

	delete[] s_RedisArray;
}

cpp_redis::client* procademy::CRedis_TLS::GetRedis()
{
	cpp_redis::client* redis = (cpp_redis::client*)TlsGetValue(s_RedisTls);

	if (redis == nullptr)
	{
		AcquireSRWLockExclusive(&s_lock);
		redis = &s_RedisArray[s_RedisIdx++];
		TlsSetValue(s_RedisTls, redis);
		ReleaseSRWLockExclusive(&s_lock);
	}

	return redis;
}
