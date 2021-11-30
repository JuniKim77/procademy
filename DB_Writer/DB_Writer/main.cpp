#include "CDBConnector.h"
#include "CDBConnector_TLS.h"
#include <string.h>
#include "CLogger.h"

int main()
{
	procademy::CDBConnector db(L"127.0.0.1", L"root", L"123456789", L"accountdb", 3306);
	WCHAR temp[10];
	MYSQL_ROW sql_row;
	bool ret;

	//for (int i = 0; i < 100; ++i)
	//{
	//	swprintf_s(temp, _countof(temp), L"test_%d", i);
	//	ret = db.Query_Save(L"INSERT INTO test.item (item_id, name, value) VALUES (%d, '%s', %d)", i, temp, i * 10);

	//	if (!ret)
	//	{
	//		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Query Error\n");
	//	}
	//}

	db.Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = 1000000;");

	while ((sql_row = db.FetchRow()) != NULL)
	{
		printf("%3s %10s %10s\n", sql_row[0], sql_row[1], sql_row[2]);
	}

	db.FreeResult();

	db.Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` LIMIT 10;");

	while ((sql_row = db.FetchRow()) != NULL)
	{
		printf("%3s %10s %10s\n", sql_row[0], sql_row[1], sql_row[2]);
	}

	db.Disconnect();

	procademy::CDBConnector_TLS::InitDBConnector_TLS(L"127.0.0.1", L"root", L"123456789", L"accountdb", 3306, 10);

	procademy::CDBConnector_TLS::Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` WHERE `accountno` = %lld;", 1000);

	while ((sql_row = procademy::CDBConnector_TLS::FetchRow()) != NULL)
	{
		printf("%3s %10s %10s\n", sql_row[0], sql_row[1], sql_row[2]);
	}

	procademy::CDBConnector_TLS::FreeResult();

	procademy::CDBConnector_TLS::Query(L"SELECT `accountno`, `userid`, `usernick` FROM `accountdb`.`account` LIMIT 10;");

	while ((sql_row = procademy::CDBConnector_TLS::FetchRow()) != NULL)
	{
		printf("%3s %10s %10s\n", sql_row[0], sql_row[1], sql_row[2]);
	}

	procademy::CDBConnector_TLS::Disconnect();

	procademy::CDBConnector_TLS::DestroyDBConnector_TLS();

	Sleep(10000);

	return 0;
}