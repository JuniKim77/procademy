#pragma once

#include "CSession.h"

namespace procademy
{
	class CMMOServer
	{
	protected:
		CMMOServer();
		virtual ~CMMOServer();
		bool Start();
		void Stop();
		void LoadInitFile(const WCHAR* fileName);
		void QuitServer();
		void SetZeroCopy(bool on);
		void SetNagle(bool on);

		virtual void AllocSessions(int num) = 0;
		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후
		virtual void OnError(int errorcode, const WCHAR* log) = 0;

	private:

		static unsigned int WINAPI MonitorThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
		static unsigned int WINAPI IocpWorkerThread(LPVOID arg);
		static unsigned int WINAPI GameThread(LPVOID arg);
		static unsigned int WINAPI AuthThread(LPVOID arg);
		static unsigned int WINAPI SendThread(LPVOID arg);
	
		void Init();
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

		void AuthLoopProc();
		void AuthReadySessionProc(CSession* session);
		void AuthCompleteRecvProc(CSession* session);
		void AuthSessionToReleaseProc(CSession* session);
		void AuthSessionToGameProc(CSession* session);
		void AuthReleaseProc(CSession* session);

		void SendLoopProc();
		void SendPacketProc(CSession* session);

		void GameLoopProc();
		void GameReadySessionProc(CSession* session);
		void GameCompleteRecvProc(CSession* session);
		void GameSessionToReleaseProc(CSession* session);
		void GameReleaseProc(CSession* session);

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
		u_short					mMaxClient = 0;
		u_short					mPort = 0;
		WCHAR					mBindIP[32];
		HANDLE					mBeginEvent = INVALID_HANDLE_VALUE;
		int						mAuthPacketLoopNum = 1;
		int						mSendPacketLoopNum = 1;
		int						mGamePacketLoopNum = 1;

		/// <summary>
		/// 모니터링 변수들
		/// </summary>
		alignas(64) DWORD		sendTPS;
		alignas(64) DWORD		recvTPS;
		alignas(64) DWORD		disconnectCount;
		DWORD					acceptTotal;
		DWORD					acceptTPS;

	protected:
		bool					mbNagle = true;
		bool					mbZeroCopy = true;
		bool					mbExit = false;
		bool					mbBegin = false;
		BYTE					mActiveThreadNum = 0;
		BYTE					mWorkerThreadNum = 0;
	};
}