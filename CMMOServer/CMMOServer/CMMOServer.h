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
		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후
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
		/// 인증 스레드 루프
		/// </summary>
		void AuthLoopProc();
		/// <summary>
		/// Client Join 알림, RecvPost 검
		/// </summary>
		/// <param name="session"></param>
		void AuthReadySessionProc(CSession* session);
		/// <summary>
		/// recvCompleteQ에서 들어온 메세지 있으면 처리
		/// </summary>
		/// <param name="session"></param>
		void AuthCompleteRecvProc(CSession* session);
		/// <summary>
		/// 세션 완료되었다면, 릴리즈 상태로 변경
		/// </summary>
		/// <param name="session"></param>
		void AuthSessionToReleaseProc(CSession* session);
		/// <summary>
		/// 게임으로 넘어갈 준비가 다 되었다면, 게임 레디 상태로 변경
		/// </summary>
		/// <param name="session"></param>
		void AuthSessionToGameProc(CSession* session);
		/// <summary>
		/// 릴리즈 처리
		/// </summary>
		/// <param name="session"></param>
		void AuthReleaseProc(CSession* session);
		/// <summary>
		/// Auth 타임아웃 체크
		/// </summary>
		/// <param name="session"></param>
		void AuthTimeoutProc(CSession* session);

		void SendLoopProc();
		void SendPacketProc(CSession* session);

		/// <summary>
		/// 게임 스레드 루프
		/// </summary>
		void GameLoopProc();
		/// <summary>
		/// 클라 게임으로 조인 알림
		/// </summary>
		/// <param name="session"></param>
		void GameReadySessionProc(CSession* session);
		/// <summary>
		/// recvCompleteQ에서 들어온 메세지 있으면 처리
		/// </summary>
		/// <param name="session"></param>
		void GameCompleteRecvProc(CSession* session);
		/// <summary>
		/// 세션 완료되었다면, 릴리즈 상태로 변경
		/// </summary>
		/// <param name="session"></param>
		void GameSessionToReleaseProc(CSession* session);
		/// <summary>
		/// 릴리즈 처리
		/// </summary>
		/// <param name="session"></param>
		void GameReleaseProc(CSession* session);
		/// <summary>
		/// Game 타임아웃 체크
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
		/// 모니터링 변수들
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