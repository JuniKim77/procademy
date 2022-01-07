#include "CDBConnector.h"
#include <string.h>
#include "CLogger.h"
#include <strsafe.h>

procademy::CDBConnector::CDBConnector(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, int iDBPort)
{
	wcscpy_s(mDBIP, _countof(mDBIP), szDBIP);
	wcscpy_s(mDBUser, _countof(mDBIP), szUser);
	wcscpy_s(mDBPassword, _countof(mDBIP), szPassword);
	wcscpy_s(mDBName, _countof(mDBIP), szDBName);
	mDBPort = iDBPort;

	Connect();
}

procademy::CDBConnector::~CDBConnector()
{
}

bool procademy::CDBConnector::Connect(void)
{
	char chIP[sizeof(mDBIP)];
	char chDBUser[sizeof(mDBUser)];
	char chDBPassword[sizeof(mDBPassword)];
	char chDBName[sizeof(mDBName)];

	WideCharToMultiByte(CP_ACP, 0, mDBIP, -1, chIP, sizeof(chIP), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, mDBUser, -1, chDBUser, sizeof(chDBUser), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, mDBPassword, -1, chDBPassword, sizeof(chDBPassword), NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, mDBName, -1, chDBName, sizeof(chDBName), NULL, NULL);

	mysql_init(&mMySQL);

	mpMySQL = mysql_real_connect(&mMySQL, chIP, chDBUser, chDBPassword, chDBName, 3306, nullptr, 0);

	if (mpMySQL == nullptr)
	{
		SaveLastError();
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Mysql connection error : %s", mLastErrorMsg);
		return false;
	}

	bool reconnect = 0; // 1;
	mysql_options(&mMySQL, MYSQL_OPT_RECONNECT, &reconnect);

	mysql_set_character_set(mpMySQL, "utf8");

	return true;
}

bool procademy::CDBConnector::Disconnect(void)
{
	mysql_close(mpMySQL);

	return true;
}

bool procademy::CDBConnector::Query(const WCHAR* szStringFormat, ...)
{
	va_list		ap;
	WCHAR		query[eQUERY_MAX_LEN];
	char		cQuery[eQUERY_MAX_LEN * 2];
	int			query_stat;
	ULONGLONG	timeBegin;
	ULONGLONG	timeEnd;

	va_start(ap, szStringFormat);
	{
		StringCchVPrintf(query, eQUERY_MAX_LEN, szStringFormat, ap);
	}
	va_end(ap);

	WideCharToMultiByte(CP_ACP, 0, query, -1, cQuery, sizeof(cQuery), NULL, NULL);

	timeBegin = GetTickCount64();
	
	query_stat = mysql_query(mpMySQL, cQuery);

	timeEnd = GetTickCount64();

	if (query_stat != 0)
	{
		SaveLastError();

		return false;
	}

	ULONGLONG dif = timeEnd - timeBegin;

	if (dif > 5000)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"[Query: %s - %dms Too Long Time]", query, dif);
	}

	mSqlResult = mysql_use_result(mpMySQL);

	return true;
}

bool procademy::CDBConnector::Query(const WCHAR* szStringFormat, va_list ap)
{
	WCHAR		query[eQUERY_MAX_LEN];
	char		cQuery[eQUERY_MAX_LEN * 2];
	int			query_stat;
	ULONGLONG	timeBegin;
	ULONGLONG	timeEnd;

	StringCchVPrintf(query, eQUERY_MAX_LEN, szStringFormat, ap);

	WideCharToMultiByte(CP_ACP, 0, query, -1, cQuery, sizeof(cQuery), NULL, NULL);

	timeBegin = GetTickCount64();

	query_stat = mysql_query(mpMySQL, cQuery);

	timeEnd = GetTickCount64();

	if (query_stat != 0)
	{
		SaveLastError();

		return false;
	}

	ULONGLONG dif = timeEnd - timeBegin;

	if (dif > 5000)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"[Query: %s - %dms Too Long Time]", query, dif);
	}

	mSqlResult = mysql_use_result(mpMySQL);

	return true;
}

bool procademy::CDBConnector::Query_Save(const WCHAR* szStringFormat, ...)
{
	va_list		ap;
	WCHAR		query[eQUERY_MAX_LEN];
	char		cQuery[eQUERY_MAX_LEN * 2];
	int			query_stat;

	va_start(ap, szStringFormat);
	{
		StringCchVPrintf(query, eQUERY_MAX_LEN, szStringFormat, ap);
	}
	va_end(ap);

	WideCharToMultiByte(CP_ACP, 0, query, -1, cQuery, sizeof(cQuery), NULL, NULL);

	query_stat = mysql_query(mpMySQL, cQuery);

	if (query_stat != 0)
	{
		SaveLastError();

		return false;
	}

	return true;
}

bool procademy::CDBConnector::Query_Save(const WCHAR* szStringFormat, va_list ap)
{
	WCHAR		query[eQUERY_MAX_LEN];
	char		cQuery[eQUERY_MAX_LEN * 2];
	int			query_stat;

	StringCchVPrintf(query, eQUERY_MAX_LEN, szStringFormat, ap);

	WideCharToMultiByte(CP_ACP, 0, query, -1, cQuery, sizeof(cQuery), NULL, NULL);

	query_stat = mysql_query(mpMySQL, cQuery);

	if (query_stat != 0)
	{
		SaveLastError();

		return false;
	}

	return true;
}

MYSQL_ROW procademy::CDBConnector::FetchRow(void)
{
	return mysql_fetch_row(mSqlResult);
}

void procademy::CDBConnector::FreeResult(void)
{
	mysql_free_result(mSqlResult);
}

void procademy::CDBConnector::SaveLastError(void)
{
	mLastError = mysql_errno(&mMySQL);
	const char* err = mysql_error(&mMySQL);

	MultiByteToWideChar(CP_ACP, 0, err, -1, mLastErrorMsg, _countof(mLastErrorMsg));
}
