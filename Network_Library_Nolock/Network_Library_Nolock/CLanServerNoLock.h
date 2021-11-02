#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "TC_LFStack.h"
#include "TC_LFQueue.h"

namespace procademy
{
	typedef u_int64 SESSION_ID;
	class CNetPacket;

	struct SessionIoCount
	{
		union
		{
			LONG ioCount = 0;
			struct
			{
				SHORT count;
				SHORT isReleased;
			} releaseCount;
		};
	};

	struct Session
	{
		WSAOVERLAPPED				recvOverlapped;
		WSAOVERLAPPED				sendOverlapped;
		RingBuffer					recvQ;
		TC_LFQueue<CNetPacket*>		sendQ;
		int							numSendingPacket = 0;
		alignas(64) SessionIoCount	ioBlock;
		alignas(64) bool			isSending;
		bool						bIsAlive;
		SOCKET						socket = INVALID_SOCKET;
		u_short						port;
		ULONG						ip;
		u_int64						sessionID;

		Session()
			: isSending(false)
			, sessionID(0)
			, bIsAlive(false)
		{
			ZeroMemory(&recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&sendOverlapped, sizeof(WSAOVERLAPPED));
		}

		Session(SOCKET _socket, ULONG _ip, u_short _port)
			: socket(_socket)
			, ip(_ip)
			, port(_port)
			, isSending(false)
			, sessionID(0)
			, bIsAlive(false)
		{
			ZeroMemory(&recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};

	class CLanServerNoLock
	{
	protected:
		CLanServerNoLock();
		~CLanServerNoLock();
		bool Start();
		void Stop();
		int GetSessionCount();
		void WaitForThreadsFin();

		bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
		void SendPacket(SESSION_ID SessionID, CNetPacket* packet); // SESSION_ID / HOST_ID

		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후

		virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept 후 접속처리 완료 후 호출.
		virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release 후 호출
		virtual void LoadInitFile(const WCHAR* fileName);

		virtual void OnRecv(SESSION_ID SessionID, CNetPacket* packet) = 0; //< 패킷 수신 완료 후
		//	virtual void OnSend(SessionID, int sendsize) = 0;           < 패킷 송신 완료 후

		//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
		//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

		virtual void OnError(int errorcode, const WCHAR* log) = 0;
		void QuitServer();

	private:
		Session* FindSession(u_int64 sessionNo);
		void InsertSessionData(Session* session);
		void DeleteSessionData(u_int64 sessionNo);
		void UpdateSessionData(u_int64 sessionNo, Session* session);
		bool CreateListenSocket();
		bool BeginThreads();
		static unsigned int WINAPI WorkerThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
		static unsigned int WINAPI MonitoringThread(LPVOID arg);
		void NetworkMonitorProc();
		bool RecvPost(Session* session, bool isAccepted = false);
		bool SendPost(Session* session);
		void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv);
		void IncrementIOProc(Session* session, int logic);
		void DecrementIOProc(Session* session, int logic);
		void ReleaseProc(Session* session);
		void AcceptProc();
		Session* CreateSession(SOCKET client, SOCKADDR_IN clientAddr);
		void CompleteMessage();
		void CompleteRecv(Session* session, DWORD transferredSize);
		void CompleteSend(Session* session, DWORD transferredSize);
		void CloseSessions();
		void InitializeEmptyIndex();
		u_int64 GenerateSessionID();
		u_short GetIndexFromSessionNo(u_int64 sessionNo);

	private:
		SOCKET				mListenSocket = INVALID_SOCKET;
		BYTE				mNumThreads = 0;
		HANDLE				mHcp = INVALID_HANDLE_VALUE;
		HANDLE* mhThreads = nullptr;
		Session* mSessionArray = nullptr;
		u_int64				mSessionIDCounter = 1;
		TC_LFStack<u_short> mEmptyIndexes;
		u_short				mPort = 0;
		WCHAR				mBindIP[32];
		BYTE				mActiveThreadNum = 0;
		BYTE				mWorkerThreadNum = 0;
		u_short				mMaxClient = 0;

		/// <summary>
		/// 모니터링 변수들
		/// </summary>
		alignas(64) DWORD	sendTPS;
		alignas(64) DWORD	recvTPS;
		alignas(64) DWORD	disconnectCount;
		DWORD				acceptTotal;
		DWORD				acceptTPS;

	protected:
		bool				mbNagle = true;
		bool				mbMonitoring = true;
		bool				mbZeroCopy = true;
		bool				mExit = false;
		bool				mBegin = false;
		HANDLE				mBeginEvent = INVALID_HANDLE_VALUE;

		struct Monitor
		{
			DWORD			prevSendTPS;
			DWORD			prevRecvTPS;
			DWORD			acceptTotal;
			DWORD			acceptTPS = 0;
		};

		/// <summary>
		/// Monitoring members
		/// </summary>
		Monitor		mMonitor;
	};
}
