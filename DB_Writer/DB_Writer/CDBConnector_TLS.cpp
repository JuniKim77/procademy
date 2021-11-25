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
