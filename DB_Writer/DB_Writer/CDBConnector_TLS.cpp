#include "CDBConnector_TLS.h"

namespace procademy
{
	CDBConnector**		CDBConnector_TLS::s_connectors;
	DWORD				CDBConnector_TLS::s_connectorTls;
	LONG				CDBConnector_TLS::s_connectorIdx;
	LONG				CDBConnector_TLS::s_connectorNum;
	SRWLOCK				CDBConnector_TLS::s_lock;
}

void procademy::CDBConnector_TLS::InitDBConnector_TLS(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, int iDBPort, int num)
{
	s_connectorTls = TlsAlloc();
	InitializeSRWLock(&s_lock);
	s_connectors = new CDBConnector*[num];
	s_connectorNum = num;

	for (int i = 0; i < num; ++i)
	{
		s_connectors[i] = new CDBConnector(szDBIP, szUser, szPassword, szDBName, iDBPort);
	}
}

void procademy::CDBConnector_TLS::DestroyDBConnector_TLS()
{
	for (int i = 0; i < s_connectorNum; ++i)
	{
		s_connectors[i]->Disconnect();
		delete s_connectors[i];
	}

	delete[] s_connectors;
}

bool procademy::CDBConnector_TLS::Connect(void)
{
	CDBConnector* conn = GetDBConnector();

	return conn->Connect();
}

bool procademy::CDBConnector_TLS::Disconnect(void)
{
	CDBConnector* conn = GetDBConnector();

	return conn->Disconnect();
}

bool procademy::CDBConnector_TLS::Query(const WCHAR* szStringFormat, ...)
{
	CDBConnector*	conn = GetDBConnector();
	va_list			ap;
	bool			ret;

	va_start(ap, szStringFormat);
	{
		ret = conn->Query(szStringFormat, ap);
	}
	va_end(ap);

	return ret;
}

bool procademy::CDBConnector_TLS::Query_Save(const WCHAR* szStringFormat, ...)
{
	CDBConnector*	conn = GetDBConnector();
	va_list			ap;
	bool			ret;

	va_start(ap, szStringFormat);
	{
		ret = conn->Query_Save(szStringFormat, ap);
	}
	va_end(ap);

	return ret;
}

MYSQL_ROW procademy::CDBConnector_TLS::FetchRow(void)
{
	CDBConnector* conn = GetDBConnector();

	return conn->FetchRow();
}

void procademy::CDBConnector_TLS::FreeResult(void)
{
	CDBConnector* conn = GetDBConnector();

	conn->FreeResult();
}

int procademy::CDBConnector_TLS::GetLastError(void)
{
	CDBConnector* conn = GetDBConnector();

	return conn->GetLastError();
}

WCHAR* procademy::CDBConnector_TLS::GetLastErrorMsg(void)
{
	CDBConnector* conn = GetDBConnector();

	return conn->GetLastErrorMsg();
}

procademy::CDBConnector* procademy::CDBConnector_TLS::GetDBConnector()
{
	CDBConnector* conn = (CDBConnector*)TlsGetValue(s_connectorTls);

	if (conn == nullptr)
	{
		AcquireSRWLockExclusive(&s_lock);
		conn = s_connectors[s_connectorIdx++];
		TlsSetValue(s_connectorTls, conn);
		ReleaseSRWLockExclusive(&s_lock);
	}

	return conn;
}
