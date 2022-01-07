#pragma once

#include "CSession.h"

namespace procademy
{
	class CMMOServer
	{
	private:
		enum {
			MAX_SESSION = 20000
		};
	protected:
		CMMOServer();
		virtual ~CMMOServer();
		bool Start();
		void Stop();
		void QuitServer();
		void SetZeroCopy(bool on);
		void SetNagle(bool on);
		void SetSession(CSession* session);

		virtual void AllocSessions(int num) = 0;
		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept ����
		virtual void OnError(int errorcode, const WCHAR* log) = 0;
		virtual void OnAuth_Update() = 0;
		virtual void OnGame_Update() = 0;

	private:
		static unsigned int WINAPI MonitorThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
		static unsigned int WINAPI IocpWorkerThread(LPVOID arg);
		static unsigned int WINAPI GameThread(LPVOID arg);
		static unsigned int WINAPI AuthThread(LPVOID arg);
		static unsigned int WINAPI SendThread(LPVOID arg);
	
		void Init();
		void LoadInitFile(const WCHAR* fileName);
		bool CreateIOCP();
		bool CreateListenSocket();
		bool BeginThreads();
		void InitializeEmptyIndex();
		void AcceptProc();
		void MonitorProc();
		void GQCSProc();
		void GameThreadProc();
		void AuthThreadProc();
		void SendThreadProc();
		void CreateSession(SOCKET client, SOCKADDR_IN clientAddr);
		void IncrementIOProc(CSession* session, int logic);
		void DecrementIOProc(CSession* session, int logic);
		void CompleteRecv(CSession* session, DWORD transferredSize);
		void CompleteSend(CSession* session, DWORD transferredSize);
		bool RecvPost(CSession* session, bool isFirst = false);
		bool SendPost(CSession* session);
		void SendPacket(CSession* session, CNetPacket* packet); // SESSION_ID / HOST_ID
		void SetWSABuf(WSABUF* bufs, CSession* session, bool isRecv);
		void ReleaseProc(CSession* session);
		void Disconnect(CSession* session);

		/// <summary>
		/// ���� ������ ����
		/// </summary>
		void AuthLoopProc();
		/// <summary>
		/// Client Join �˸�, RecvPost ��
		/// </summary>
		/// <param name="session"></param>
		void AuthReadySessionProc(CSession* session);
		/// <summary>
		/// recvCompleteQ���� ���� �޼��� ������ ó��
		/// </summary>
		/// <param name="session"></param>
		void AuthCompleteRecvProc(CSession* session);
		/// <summary>
		/// ���� �Ϸ�Ǿ��ٸ�, ������ ���·� ����
		/// </summary>
		/// <param name="session"></param>
		void AuthSessionToReleaseProc(CSession* session);
		/// <summary>
		/// �������� �Ѿ �غ� �� �Ǿ��ٸ�, ���� ���� ���·� ����
		/// </summary>
		/// <param name="session"></param>
		void AuthSessionToGameProc(CSession* session);
		/// <summary>
		/// ������ ó��
		/// </summary>
		/// <param name="session"></param>
		void AuthReleaseProc(CSession* session);
		/// <summary>
		/// Auth Ÿ�Ӿƿ� üũ
		/// </summary>
		/// <param name="session"></param>
		void AuthTimeoutProc(CSession* session);

		void SendLoopProc();
		void SendPacketProc(CSession* session);

		/// <summary>
		/// ���� ������ ����
		/// </summary>
		void GameLoopProc();
		/// <summary>
		/// Ŭ�� �������� ���� �˸�
		/// </summary>
		/// <param name="session"></param>
		void GameReadySessionProc(CSession* session);
		/// <summary>
		/// recvCompleteQ���� ���� �޼��� ������ ó��
		/// </summary>
		/// <param name="session"></param>
		void GameCompleteRecvProc(CSession* session);
		/// <summary>
		/// ���� �Ϸ�Ǿ��ٸ�, ������ ���·� ����
		/// </summary>
		/// <param name="session"></param>
		void GameSessionToReleaseProc(CSession* session);
		/// <summary>
		/// ������ ó��
		/// </summary>
		/// <param name="session"></param>
		void GameReleaseProc(CSession* session);
		/// <summary>
		/// Game Ÿ�Ӿƿ� üũ
		/// </summary>
		/// <param name="session"></param>
		void GameTimeoutProc(CSession* session);

	private:
		enum {
			SEND_BUF_SIZE = 8192
		};

		SOCKET					mListenSocket = INVALID_SOCKET;
		BYTE					mNumThreads = 0;
		HANDLE					mIOCP = INVALID_HANDLE_VALUE;
		HANDLE*					mhThreads = nullptr;
		CSession**				mSessionArray = nullptr;
		TC_LFStack<u_short>		mEmptyIndexes;
		u_int64					mSessionIDCounter = 1;
		u_short					mSessionCount = 0;
		u_short					mPort = 0;
		WCHAR					mBindIP[32];
		HANDLE					mAcceptEvent = INVALID_HANDLE_VALUE;
		HANDLE					mGameEvent = INVALID_HANDLE_VALUE;
		HANDLE					mAuthEvent = INVALID_HANDLE_VALUE;
		HANDLE					mSendEvent = INVALID_HANDLE_VALUE;
		int						mAuthPacketLoopNum = 1;
		int						mGamePacketLoopNum = 300;
		int						mTimeOut;

		/// <summary>
		/// ����͸� ������
		/// </summary>
		alignas(64) DWORD		sendTPS;
		alignas(64) DWORD		recvTPS;
		DWORD					acceptTotal;
		DWORD					acceptTPS;

	protected:
		DWORD					sendLoopCount = 0;
		DWORD					authLoopCount = 0;
		DWORD					gameLoopCount = 0;
		alignas(64) DWORD		joinCount = 0;
		bool					mbNagle = true;
		bool					mbZeroCopy = true;
		bool					mbExit = false;
		bool					mbBegin = false;
		BYTE					mActiveThreadNum = 0;
		BYTE					mWorkerThreadNum = 0;
		u_short					mMaxClient = 0;
		int						mMaxTransferToAuth = 20;
		int						mMaxTransferToGame = 20;
		int						mAuthPlayerNum = 0;
		int						mGamePlayerNum = 0;

		struct Monitor
		{
			DWORD			prevSendTPS;
			DWORD			prevRecvTPS;
			DWORD			acceptTotal;
			DWORD			acceptTPS = 0;
			DWORD			prevSendLoopCount = 0;
			DWORD			prevAuthLoopCount = 0;
			DWORD			prevGameLoopCount = 0;
		};

		/// <summary>
		/// Monitoring members
		/// </summary>
		Monitor		mMonitor;
	};
}