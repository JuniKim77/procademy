#pragma once

#include "CDBConnector.h"

namespace procademy
{
	class CDBConnector_TLS
	{
	public:
		static void				InitDBConnector_TLS(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, int iDBPort, int num);
		static void				DestroyDBConnector_TLS();
		static CDBConnector*	GetDBConnector();

	private:
		static CDBConnector**	s_connectors;
		static DWORD			s_connectorTls;
		static LONG				s_connectorIdx;
		static LONG				s_connectorNum;
		static SRWLOCK			s_lock;
	};
}
