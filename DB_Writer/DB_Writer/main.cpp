#include "CRedis_TLS.h"
#include "CDBConnector_TLS.h"
#include "CLogger.h"
#include <wtypes.h>
#include <process.h>
#include "CProfiler.h"
#include <time.h>
#include <queue>
#include <string>

#define dfTHREAD_NUM (20)
#define dfCHUNK_SIZE (1000)

unsigned int WINAPI workerThread(LPVOID arg); 

int g_counter = 0;

int main()
{
	srand((unsigned int)time(NULL));
	procademy::CDBConnector db(L"127.0.0.1", L"root", L"123456789", L"accountdb", 3306);
	MYSQL_ROW sql_row;
	cpp_redis::client	curRedis;
	char				szAccountNumber[20] = { 0, };
	char				value[20] = { 0, };
	int					result = 0;
	char				buffer[12] = { 0, };
	int					num = 1;
	int					count = 0;

	//CProfiler::Begin(L"SINGLE_DB_SELECT");
	curRedis.connect();
	CProfiler::Begin(L"SINGLE_DB_REDIS_SELECT");

	while (count < (dfCHUNK_SIZE * dfTHREAD_NUM))
	{
		if (num > 5000)
			num = 1;

		db.Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %d;", num);

		if ((sql_row = db.FetchRow()) != NULL)
		{
			result++;

			_itoa_s(result, value, 20, 10);
			strcpy_s(szAccountNumber, 20, sql_row[0]);

			curRedis.setex(szAccountNumber, 30, value);
		}

		db.FreeResult();

		count++;
		num++;
	}
	CProfiler::End(L"SINGLE_DB_REDIS_SELECT");
	//CProfiler::End(L"SINGLE_DB_SELECT");

	//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Main Count: %d | Result: %d\n", count, result);

	db.Disconnect();
	curRedis.disconnect();

	procademy::CDBConnector_TLS::InitDBConnector_TLS(L"127.0.0.1", L"root", L"123456789", L"accountdb", 3306, dfTHREAD_NUM);
	procademy::CRedis_TLS::InitRedis_TLS(L"127.0.0.1", 6379, dfTHREAD_NUM);

	HANDLE handles[dfTHREAD_NUM];

	CProfiler::Begin(L"MULTI_DB_REDIS_SELECT");
	//CProfiler::Begin(L"MULTI_DB_SELECT");

	for (int i = 0; i < dfTHREAD_NUM; ++i)
	{
		handles[i] = (HANDLE)_beginthreadex(nullptr, 0, workerThread, 0, 0, nullptr);
	}

	DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, handles, true, INFINITE);

	CProfiler::End(L"MULTI_DB_REDIS_SELECT");

	CProfiler::Print();

	switch (retval)
	{
	case WAIT_FAILED:
		wprintf_s(L"Main Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		wprintf_s(L"Main Thread Timeout Error\n");
		break;
	case WAIT_OBJECT_0:
		wprintf_s(L"None Error\n");
		break;
	default:
		break;
	}

	procademy::CDBConnector_TLS::DestroyDBConnector_TLS();
	procademy::CRedis_TLS::DestroyRedis_TLS();

	return 0;
}

unsigned int __stdcall workerThread(LPVOID arg)
{
	MYSQL_ROW			sql_row;
	cpp_redis::client*	redis = nullptr;
	char				szAccountNumber[20] = { 0, };
	char				value[20] = { 0, };
	int					count = 0;
	int					result = 0;
	char				buffer[12] = { 0, };
	int					num = 1;

	while (count < dfCHUNK_SIZE)
	{
		procademy::CDBConnector_TLS::Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %d;", num);

		while ((sql_row = procademy::CDBConnector_TLS::FetchRow()) != NULL)
		{
			result++;

			_itoa_s(result, value, 20, 10);
			std::string str(value);
			strcpy_s(szAccountNumber, 20, sql_row[0]);

			procademy::CRedis_TLS::SetRedis(szAccountNumber, 30, str);
		}

		procademy::CDBConnector_TLS::FreeResult();

		count++;
		num++;
	}

	int temp = 1;
	int countReids = 0;

	count = 0;
	
	while (count < dfCHUNK_SIZE)
	{
		_itoa_s(temp, szAccountNumber, 20, 10);
		_itoa_s(temp, buffer, 12, 10);

		redis = procademy::CRedis_TLS::GetRedis();

		redis->get(szAccountNumber, [szAccountNumber, buffer, &countReids](cpp_redis::reply& reply) {
			if (reply.is_string())
			{
				if (strcmp(reply.as_string().c_str(), buffer) == 0)
				{
					countReids++;
				}
			}
			});

		redis->sync_commit();

		temp++;
		count++;
	}

	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Thread Num: %u -- Main Count: %d\n", GetCurrentThreadId(), countReids);

	//CProfiler::End(L"MULTI_DB_SELECT");

	//CProfiler::SetRecord(L"MULTI_DB_SELECT", end.QuadPart - begin.QuadPart, CProfiler::PROFILE_TYPE::MICRO_SECONDS);

	//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Thread Num: %u -- Main Count: %d | Result: %d\n", GetCurrentThreadId(), count, result);

	procademy::CDBConnector_TLS::Disconnect();

	return 0;
}
