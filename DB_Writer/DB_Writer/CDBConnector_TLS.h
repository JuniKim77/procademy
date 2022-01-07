#pragma once

#include "CDBConnector.h"

namespace procademy
{
	class CDBConnector_TLS
	{
	public:
		static void				InitDBConnector_TLS(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, int iDBPort, int num);
		static void				DestroyDBConnector_TLS();
		static bool				Connect(void);
		static bool				Disconnect(void);
		static bool				Query(const WCHAR* szStringFormat, ...);
		static bool				Query_Save(const WCHAR* szStringFormat, ...);
		static MYSQL_ROW		FetchRow(void);
		static void				FreeResult(void);
		static int				GetLastError(void);
		static WCHAR*			GetLastErrorMsg(void);

	private:
		static CDBConnector*	GetDBConnector();

	private:
		static CDBConnector**	s_connectors;
		static DWORD			s_connectorTls;
		static LONG				s_connectorIdx;
		static LONG				s_connectorNum;
		static SRWLOCK			s_lock;
	};
}
