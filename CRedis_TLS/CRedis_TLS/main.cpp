#include "CRedis_TLS.h"

int main()
{
	procademy::CRedis_TLS::InitRedis_TLS(L"127.0.0.1", 6379, 1);

	cpp_redis::client* redis = procademy::CRedis_TLS::GetRedis();

	char numbers[10];

	for (int i = 0; i < 1000; ++i)
	{
		_itoa_s(i, numbers, 10, 10);
		redis->set(numbers, "1");

		redis->get(numbers, [numbers](cpp_redis::reply& reply) {
			if (reply.is_string())
			{
				if (reply.as_string() != "1")
					printf_s("Not Mached %s\n", reply.as_string().c_str());
				else
					printf_s("Mached %s - %s\n", numbers, reply.as_string().c_str());
			}
			});

		redis->sync_commit();
	}

	procademy::CRedis_TLS::DestroyRedis_TLS();

	return 0;
}