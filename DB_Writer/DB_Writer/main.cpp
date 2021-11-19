#include "CDBConnector.h"
#include <string.h>
#include "CLogger.h"

int main()
{
	procademy::CDBConnector db(L"127.0.0.1", L"root", L"123456789", L"test", 3306);
	WCHAR temp[10];
	MYSQL_ROW sql_row;
	bool ret;

	/*for (int i = 0; i < 100; ++i)
	{
		swprintf_s(temp, _countof(temp), L"test_%d", i);
		ret = db.Query_Save(L"INSERT INTO test.item (item_id, name, value) VALUES (%d, '%s', %d)", i, temp, i * 10);

		if (!ret)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Query Error\n");
		}
	}*/

	db.Query(L"SELECT * FROM test.item");

	while ((sql_row = db.FetchRow()) != NULL)
	{
		printf("%3s %10s %s\n", sql_row[0], sql_row[1], sql_row[2]);
	}

	db.Disconnect();

	return 0;
}