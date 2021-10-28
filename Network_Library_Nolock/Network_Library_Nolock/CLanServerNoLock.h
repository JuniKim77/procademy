#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
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
		~CLanServerNoLock();
		bool Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
		bool Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);
		void Stop();
		int GetSessionCount();
		void WaitForThreadsFin();

		bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
		void SendPacket(SESSION_ID SessionID, CNetPacket* packet); // SESSION_ID / HOST_ID

		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후

		virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept 후 접속처리 완료 후 호출.
		virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release 후 호출


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
		bool AcceptProc();
		Session* CreateSession(SOCKET client, SOCKADDR_IN clientAddr);
		bool OnCompleteMessage();
		void CompleteRecv(Session* session, DWORD transferredSize);
		void CompleteSend(Session* session, DWORD transferredSize);
		void CloseSessions();
		void InitializeEmptyIndex();
		u_int64 GenerateSessionID();
		u_short GetIndexFromSessionNo(u_int64 sessionNo);

	private:
		/// <summary>
		/// Listen Socket Info
		/// </summary>
		u_short				mPort = 0;
		u_long				mBindIP = 0;
		SOCKET				mListenSocket;

		/// <summary>
		/// Options
		/// </summary>
		BYTE				mMaxRunThreadSize = 0;
		BYTE				mWorkerThreadSize = 0;
		u_short				mMaxClient = 0;

		/// <summary>
		/// Network Status
		/// </summary>
		bool				mbIsRunning = false;
		BYTE				mNumThreads = 0;
		bool				mbIsQuit = false;

		/// <summary>
		/// Handles
		/// </summary>
		HANDLE				mHcp;
		HANDLE*				mhThreads;

		/// <summary>
		/// Session Objects
		/// </summary>
		Session*			mSessionArray;
		u_int64				mSessionIDCounter = 1;
		TC_LFStack<u_short> mEmptyIndexes;

	protected:
		bool				mbNagle = true;
		bool				mbMonitoring = true;
		bool				mbZeroCopy = true;

		struct Monitor
		{
			alignas(64) DWORD	sendTPS;
			alignas(64) DWORD	recvTPS;
			alignas(64) DWORD	acceptCount;
			alignas(64) DWORD	disconnectCount;
			DWORD prevSendTPS;
			DWORD prevRecvTPS;
		};

		/// <summary>
		/// Monitoring members
		/// </summary>
		alignas(64) Monitor		mMonitor;
	};
}

