#pragma once
/////////////////////////////////////////////////////////
// MySQL DB 연결 클래스
//
// 단순하게 MySQL Connector 를 통한 DB 연결만 관리한다.
//
// 스레드에 안전하지 않으므로 주의 해야 함.
// 여러 스레드에서 동시에 이를 사용한다면 개판이 됨.
//
/////////////////////////////////////////////////////////
#pragma (lib, "mysqlclient.lib")

#include <Windows.h>
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"
#include "TC_LFQueue.h"

namespace procademy
{
	class CDBConnector
	{
	public:

		enum en_DB_CONNECTOR
		{
			eQUERY_MAX_LEN = 2048

		};

		// 연결시도
		CDBConnector(const WCHAR* szDBIP, const WCHAR* szUser, const WCHAR* szPassword, const WCHAR* szDBName, int iDBPort);
		virtual		~CDBConnector();

		//////////////////////////////////////////////////////////////////////
		// MySQL DB 연결
		//////////////////////////////////////////////////////////////////////
		bool		Connect(void);

		//////////////////////////////////////////////////////////////////////
		// MySQL DB 끊기 끌때 꼭 할 것
		//////////////////////////////////////////////////////////////////////
		bool		Disconnect(void);


		//////////////////////////////////////////////////////////////////////
		// 쿼리 날리고 결과셋 임시 보관
		//
		//////////////////////////////////////////////////////////////////////
		bool		Query(WCHAR* szStringFormat, ...);
		bool		Query_Save(WCHAR* szStringFormat, ...);	// DBWriter 스레드의 Save 쿼리 전용
																// 결과셋을 저장하지 않음.

		//////////////////////////////////////////////////////////////////////
		// 쿼리를 날린 뒤에 결과 뽑아오기.
		//
		// 결과가 없다면 NULL 리턴.
		//////////////////////////////////////////////////////////////////////
		MYSQL_ROW	FetchRow(void);

		//////////////////////////////////////////////////////////////////////
		// 한 쿼리에 대한 결과 모두 사용 후 정리.
		//////////////////////////////////////////////////////////////////////
		void		FreeResult(void);


		//////////////////////////////////////////////////////////////////////
		// Error 얻기.한 쿼리에 대한 결과 모두 사용 후 정리.
		//////////////////////////////////////////////////////////////////////
		int			GetLastError(void) { return mLastError; };
		WCHAR*		GetLastErrorMsg(void) { return mLastErrorMsg; }


	private:

		//////////////////////////////////////////////////////////////////////
		// mysql 의 LastError 를 맴버변수로 저장한다.
		//////////////////////////////////////////////////////////////////////
		void		SaveLastError(void);

	private:



		//-------------------------------------------------------------
		// MySQL 연결객체 본체
		//-------------------------------------------------------------
		MYSQL		mMySQL;

		//-------------------------------------------------------------
		// MySQL 연결객체 포인터. 위 변수의 포인터임. 
		// 이 포인터의 null 여부로 연결상태 확인.
		//-------------------------------------------------------------
		MYSQL*		mpMySQL = nullptr;

		//-------------------------------------------------------------
		// 쿼리를 날린 뒤 Result 저장소.
		//
		//-------------------------------------------------------------
		MYSQL_RES*	mSqlResult = nullptr;

		WCHAR		mDBIP[16];
		WCHAR		mDBUser[64];
		WCHAR		mDBPassword[64];
		WCHAR		mDBName[64];
		int			mDBPort = 0;


		WCHAR		mQuery[eQUERY_MAX_LEN];
		char		mQueryUTF8[eQUERY_MAX_LEN];

		int			mLastError = 0;
		WCHAR		mLastErrorMsg[128];

	};
}
















