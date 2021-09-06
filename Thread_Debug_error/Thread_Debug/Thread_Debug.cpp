// Thread_Debug.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "define.h"

////////////////////////////////////////////////////////
// ������ ��
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// ���ӿ�û ���. 
//
// IOThread ���� �ֱ������� ����, 
// AcceptThread ���� �� ����Ʈ�� ��(SessionID ��)�� �̾Ƽ�  ���ο� Session �� �����.
// �̹� �����ϴ� SessionID �� ���ð�� ���� �ؾ� ��
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Accept_cs;
list<DWORD>			g_AcceptPacketList;

#define				LockAccept()	EnterCriticalSection(&g_Accept_cs)
#define 			UnlockAccept()	LeaveCriticalSection(&g_Accept_cs)

////////////////////////////////////////////////////////
// �׼� ��û ���.
//
// IOThread ���� �ֱ������� ����(SessionID),
// UpdateThread ������ �� ���� �̾Ƽ� �ش� �÷��̾��� Content + 1 �� �Ѵ�.
// �������� �ʴ� SessionID �� ������ �����ؾ� ��.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Action_cs;
list<DWORD>			g_ActionPacketList;

#define				LockAction()	EnterCriticalSection(&g_Action_cs)
#define				UnlockAction()	LeaveCriticalSection(&g_Action_cs)

////////////////////////////////////////////////////////
// �������� ��û ���
//
// IOThread ���� �ֱ������� ����, 
// AcceptThread ���� �� ����Ʈ�� ���� �̾Ƽ� (SessionID ��) �ش� Session �� ���� ��Ų��.
////////////////////////////////////////////////////////
CRITICAL_SECTION	g_Disconnect_cs;
list<DWORD>			g_DisconnectPacketList;

#define				LockDisconnect()	EnterCriticalSection(&g_Disconnect_cs)
#define				UnlockDisconnect()	LeaveCriticalSection(&g_Disconnect_cs)

////////////////////////////////////////////////////////
// Session ���.
//
// ������ �Ϸ�� (Accept ó�� �Ϸ�)  st_SESSION �� ���� �����Ͽ�, SessionList �� �����͸� �ִ´�.
// �׸��� ������ ������ �� �ش� ������ ���� �Ѵ�.
////////////////////////////////////////////////////////
//CRITICAL_SECTION		g_Session_cs;
//list<st_SESSION *>		g_SessionList;

//#define	LockSession()	EnterCriticalSection(&g_Session_cs)
//#define UnlockSession()	LeaveCriticalSection(&g_Session_cs)

////////////////////////////////////////////////////////
// Player ���.
//
// Session �� ���� ��, ���� �ɶ� (Accept ó�� �Ϸ��)  st_PLAYER ��ü�� �Բ� �����Ǿ� ���⿡ ��� �ȴ�.
////////////////////////////////////////////////////////
//CRITICAL_SECTION		g_Player_cs;
//list<st_PLAYER *>		g_PlayerList;
int g_SessionStatus[dfSESSION_NUM] = { 0, };
st_SESSION* g_SessionArray[dfSESSION_NUM] = { nullptr, };
st_PLAYER* g_PlayerArray[dfSESSION_NUM] = { nullptr, };


//void LockPlayer()
//{
//	EnterCriticalSection(&g_Player_cs);
//}
//
//void UnlockPlayer()
//{
//	LeaveCriticalSection(&g_Player_cs);
//}


HANDLE	g_hExitThreadEvent;

HANDLE	g_hAcceptThreadEvent;
HANDLE	g_hUpdateThreadEvent;

WCHAR *g_szDebug;

void NewSession(DWORD dwSessionID)
{
	st_SESSION *pSession = new st_SESSION;
	pSession->SessionID = dwSessionID;
	
	//LockSession();
	//g_SessionList.push_back(pSession);
	//UnlockSession();


	st_PLAYER *pPlayer = new st_PLAYER;
	pPlayer->SessionID = dwSessionID;
	memset(pPlayer->Content, 0, sizeof(pPlayer->Content));

	//LockPlayer();
	//g_PlayerList.push_back(pPlayer);
	//UnlockPlayer();
}

void ConnectSession(DWORD dwSessionID)
{
	for (int i = 0; i < dfSESSION_NUM; ++i)
	{
		if (g_SessionStatus[i] == dfSESSION_NONE)
		{
			if (g_SessionArray[i] == nullptr)
			{
				g_SessionArray[i] = new st_SESSION;
			}

			g_SessionArray[i]->SessionID = dwSessionID;
			g_SessionStatus[i] = dfSESSION_CONNECT;
			break;
		}
	}
}

void DeleteSession(DWORD dwSessionID)
{
	//LockSession();
	//list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();
	//for ( ; SessionIter != g_SessionList.end(); SessionIter++ )
	//{
	//	if ( dwSessionID == (*SessionIter)->SessionID )
	//	{
	//		delete *SessionIter;
	//		g_SessionList.erase(SessionIter);
	//		break;
	//	}
	//}
	////UnlockSession();

	////LockPlayer();
	//list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
	//for ( ; PlayerIter != g_PlayerList.end(); PlayerIter++ )
	//{
	//	if ( dwSessionID == (*PlayerIter)->SessionID )
	//	{
	//		delete *PlayerIter;
	//		g_PlayerList.erase(PlayerIter);
	//		break;
	//	}
	//}
	//UnlockPlayer();
}


bool FindSessionList(DWORD dwSessionID)
{
	//LockSession();

	//list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();
	//for ( ; SessionIter != g_SessionList.end(); SessionIter++ )
	//{
	//	if ( dwSessionID == (*SessionIter)->SessionID )
	//	{
	//		//UnlockSession();
	//		return true;
	//	}
	//}
	//UnlockSession();

	for (int i = 0; i < dfSESSION_NUM; ++i)
	{
		if (g_SessionStatus[i] != dfSESSION_NONE && g_SessionArray[i]->SessionID == dwSessionID)
		{
			return true;
		}
	}

	return false;
}

void UpdateSession(DWORD dwSessionID)
{
	for (int i = 0; i < dfSESSION_NUM; ++i)

	{
		if (g_SessionStatus[i] == dfSESSION_CONNECT)
		{
			if (g_PlayerArray[i] == nullptr)
			{
				g_PlayerArray[i] = new st_PLAYER;
			}

			g_PlayerArray[i]->SessionID = g_SessionArray[i]->SessionID;
			memset(g_PlayerArray[i]->Content, 0, sizeof(g_PlayerArray[i]->Content));
			g_SessionStatus[i] = dfSESSION_LOGIN;
		}

		if (g_SessionStatus[i] == dfSESSION_LOGIN && g_PlayerArray[i]->SessionID == dwSessionID)
		{
			for (int iCnt = 0; iCnt < 3; iCnt++)
			{
				g_PlayerArray[i]->Content[iCnt]++;
			}
			wprintf(L"UpdateThread - Session[%d] Content[%d] \n", dwSessionID, g_PlayerArray[i]->Content[0]);
		}
	}
}

void DisconnectSession(DWORD dwSessionID)
{
	for (int i = 0; i < dfSESSION_NUM; ++i)
	{
		if (g_SessionStatus[i] == dfSESSION_LOGIN && g_SessionArray[i]->SessionID == dwSessionID)
		{
			g_SessionStatus[i] = dfSESSION_NONE;
			wprintf(L"DisconnectThread - Session[%d]\n", dwSessionID);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Accept Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI AcceptThread(LPVOID lpParam)
{
	HANDLE hEvent[2] = {g_hExitThreadEvent, g_hAcceptThreadEvent};
	DWORD dwError;
	DWORD dwSessionID;
	bool bLoop = true;
	bool bFindIt = false;
	
	wprintf(L"Accept Thread Start\n");
	srand(GetTickCount() + 1);

	while ( bLoop )
	{
		dwError = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

		if ( dwError == WAIT_FAILED )
 		{
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			wprintf(L"Accept Thread Event Error\n");
			break;
		}

		if (dwError == WAIT_OBJECT_0)
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}


		//----------------------------------------------------------
		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		//----------------------------------------------------------

		//----------------------------------------------------------
		// ���ӿ�û ó��
		//----------------------------------------------------------
		while ( !g_AcceptPacketList.empty() )
		{
			dwSessionID = *g_AcceptPacketList.begin();
			LockAccept();
			g_AcceptPacketList.pop_front();
			UnlockAccept();
			//OutputDebugString(L"Read AcceptPacket\n");

			//----------------------------------------------------------
			// SessionList �� �̹� �����ϴ� SessionID ���� Ȯ��.  ���� ��츸 ���.
			//----------------------------------------------------------
			if ( !FindSessionList(dwSessionID) )
			{
				/*NewSession(dwSessionID);
				wprintf(L"AcceptThread - New Session[%d]\n", dwSessionID);*/
				ConnectSession(dwSessionID);
				wprintf(L"AcceptThread - New Session[%d]\n", dwSessionID);
			}
		}

		//----------------------------------------------------------
		// �������� ó��
		//----------------------------------------------------------
		//while ( !g_DisconnectPacketList.empty() )
		//{
		//	LockDisconnect();
		//	dwSessionID = *g_DisconnectPacketList.begin();
		//	g_DisconnectPacketList.pop_front();
		//	UnlockDisconnect();
		//	//OutputDebugString(L"Read DisconnectPacket\n");

		//	//----------------------------------------------------------
		//	// SessionList �� �����ϴ� SessionID ���� Ȯ��.  �ִ� ��츸 ����
		//	//----------------------------------------------------------
		//	if ( FindSessionList(dwSessionID) )
		//	{
		//		DeleteSession(dwSessionID);
		//		wprintf(L"AcceptThread - Delete Session[%d]\n", dwSessionID);
		//	}
		//}
	}

	wprintf(L"Accept Thread Exit\n");
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// IO Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI IOThread(LPVOID lpParam)
{
	DWORD dwError;
	bool bLoop = true;
	int iRand;
	DWORD dwSessionID;

	srand(GetTickCount() + 2);
	wprintf(L"IO Thread Start\n");

	while ( bLoop )
	{
		dwError = WaitForSingleObject(g_hExitThreadEvent, 10);
		if (dwError != WAIT_TIMEOUT)
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}

		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		iRand = rand() % 3;
		dwSessionID = rand() % dfSESSION_NUM;
		

		switch ( iRand )
		{
		case 0:			// Accept �߰�
			//wsprintf(g_szDebug, L"# IOThread AcceptPacket Insert [%d] \n", dwSessionID);
			LockAccept();			
			g_AcceptPacketList.push_back(dwSessionID);
			UnlockAccept();
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 1:			// Disconnect �߰�
			//wsprintf(g_szDebug, L"# IOThread DisconnetPacket Insert [%d] \n", dwSessionID);
			LockDisconnect();			
			g_DisconnectPacketList.push_back(dwSessionID);
			//UnlockDisconnect();
			SetEvent(g_hAcceptThreadEvent);
			break;

		case 2:			// Action �߰�
			//wsprintf(g_szDebug, L"# IOThread ActionPacket Insert [%d] \n", dwSessionID);
			LockAction();
			g_ActionPacketList.push_back(dwSessionID);
			//UnlockAction();
			SetEvent(g_hUpdateThreadEvent);
			break;
		}
		//OutputDebugString(g_szDebug);
	}


	wprintf(L"IO Thread Exit\n");
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Update Thread
/////////////////////////////////////////////////////////////////////////////////////
unsigned int WINAPI UpdateThread(LPVOID lpParam)
{
	HANDLE hEvent[2] = {g_hExitThreadEvent, g_hUpdateThreadEvent};
	DWORD dwError;
	DWORD dwSessionID;
	st_PLAYER *pPlayer;
	bool bLoop = true;

	srand(GetTickCount() + 3);

	wprintf(L"Update Thread Start\n");

	while ( bLoop )
	{
		dwError = WaitForMultipleObjects(2, hEvent, FALSE, INFINITE);

		if ( dwError == WAIT_FAILED )
 		{
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			wprintf(L"Update Thread Event Error\n");
			break;
		}

		if (dwError == WAIT_OBJECT_0)
		{
			SetEvent(g_hExitThreadEvent);
			break;
		}

		//----------------------------------------------------------
		// ���� ����ó�� 
		//----------------------------------------------------------
		//----------------------------------------------------------
		// �÷��̾� �׼� ó��
		//----------------------------------------------------------
		while ( !g_ActionPacketList.empty() )
		{
			LockAction();
			dwSessionID = *g_ActionPacketList.begin();
			g_ActionPacketList.pop_front();
			UnlockAction();
			//OutputDebugString(L"Read UpdatePacket\n");

			//----------------------------------------------------------
			// PlayerList �� �̹� �����ϴ� SessionID ���� Ȯ��. �ִ� ��츸 �ش� �÷��̾� ã�Ƽ� + 1
			//----------------------------------------------------------
			//LockPlayer();
			//list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
			//for ( ; PlayerIter != g_PlayerList.end(); PlayerIter++ )
			//{
			//	pPlayer = *PlayerIter;
			//	if ( dwSessionID == pPlayer->SessionID )
			//	{
			//		// ������ ������Ʈ - Content �迭���� + 1 �� ���
			//		for ( int iCnt = 0; iCnt < 3; iCnt++ )
			//		{
			//			pPlayer->Content[iCnt]++;
			//		}
			//		wprintf(L"UpdateThread - Session[%d] Content[%d] \n", dwSessionID, pPlayer->Content[0]);
			//		break;
			//	}
			//}
			//UnlockPlayer();
			UpdateSession(dwSessionID);
		}

		// ���� ���� ó��
		while (!g_DisconnectPacketList.empty())
		{
			LockDisconnect();
			dwSessionID = *g_DisconnectPacketList.begin();
			g_DisconnectPacketList.pop_front();
			UnlockDisconnect();

			DisconnectSession(dwSessionID);
		}

	}

	wprintf(L"Update Thread Exit\n");
	return 0;
}




/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Initial
/////////////////////////////////////////////////////////////////////////////////////
void Initial()
{
	g_szDebug = new WCHAR[100];

	//------------------------------------------------
	// ������ �����带 ���� �̺�Ʈ
	//------------------------------------------------
	g_hAcceptThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hUpdateThreadEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);

	//------------------------------------------------
	// ��� �����带 ���� ��ų �̺�Ʈ
	//------------------------------------------------
	g_hExitThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	InitializeCriticalSection(&g_Accept_cs);
	InitializeCriticalSection(&g_Action_cs);
	InitializeCriticalSection(&g_Disconnect_cs);
	//InitializeCriticalSection(&g_Player_cs);
	//InitializeCriticalSection(&g_Session_cs);

}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Release
/////////////////////////////////////////////////////////////////////////////////////
void Release()
{
	g_AcceptPacketList.clear();
	g_ActionPacketList.clear();
	g_DisconnectPacketList.clear();


	/*list<st_SESSION *>::iterator SessionIter = g_SessionList.begin();
	while ( SessionIter != g_SessionList.end() )
	{
		delete *SessionIter;
		SessionIter = g_SessionList.erase(SessionIter);
	}

	list<st_PLAYER *>::iterator PlayerIter = g_PlayerList.begin();
	while ( PlayerIter != g_PlayerList.end() )
	{
		delete *PlayerIter;
		PlayerIter = g_PlayerList.erase(PlayerIter);
	}*/

	for (int i = 0; i < dfSESSION_NUM; ++i)
	{
		if (g_SessionArray[i] != nullptr)
		{
			delete g_SessionArray[i];
		}

		if (g_PlayerArray[i] != nullptr)
		{
			delete g_PlayerArray[i];
		}
	}

	delete[] g_szDebug;

	DeleteCriticalSection(&g_Accept_cs);
	DeleteCriticalSection(&g_Action_cs);
	DeleteCriticalSection(&g_Disconnect_cs);
	//DeleteCriticalSection(&g_Player_cs);
	//DeleteCriticalSection(&g_Session_cs);


}
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hIOThread;			// ���ӿ�û, �����û, �׼ǿ�û �߻�.  (IO �ùķ��̼�)
	HANDLE hAcceptThread;		// ���ӿ�û, ���⿡ ���� ó��	
	HANDLE hUpdateThread;		// �׼ǿ�û ó��.

	DWORD dwThreadID;

	Initial();



	//------------------------------------------------
	// ������ ����.
	//------------------------------------------------
	hAcceptThread	= (HANDLE)_beginthreadex(NULL, 0, AcceptThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hIOThread		= (HANDLE)_beginthreadex(NULL, 0, IOThread,	(LPVOID)0, 0, (unsigned int *)&dwThreadID);
	hUpdateThread	= (HANDLE)_beginthreadex(NULL, 0, UpdateThread, (LPVOID)0, 0, (unsigned int *)&dwThreadID);


	WCHAR ControlKey;

	//------------------------------------------------
	// ���� ��Ʈ��...
	//------------------------------------------------
	while ( 1 )
	{	
		ControlKey = _getwch();
		if ( ControlKey == L'q' || ControlKey == L'Q' )
		{
			//------------------------------------------------
			// ����ó��
			//------------------------------------------------
			SetEvent(g_hExitThreadEvent);
			break;
		}
	}


	//------------------------------------------------
	// ������ ���� ���
	//------------------------------------------------
	HANDLE hThread[3] = {hAcceptThread, hIOThread, hUpdateThread};
	DWORD retval = WaitForMultipleObjects(dfTHREAD_NUM, hThread, TRUE, INFINITE);

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

	Release();


	//------------------------------------------------
	// ������ �ڵ�  ������ �������� Ȯ��.
	//------------------------------------------------
	DWORD ExitCode;

	wprintf(L"\n\n--- THREAD CHECK LOG -----------------------------\n\n");

	GetExitCodeThread(hAcceptThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - Accept Thread not exit\n");

	GetExitCodeThread(hIOThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - IO Thread not exit\n");

	GetExitCodeThread(hUpdateThread, &ExitCode);
	if ( ExitCode != 0 )
		wprintf(L"error - Update Thread not exit\n");

	return 0;
}
