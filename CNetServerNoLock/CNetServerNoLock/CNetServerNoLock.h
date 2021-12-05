#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "TC_LFStack.h"
#include "TC_LFQueue.h"

//#define PROFILE

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
		SOCKET						socket = INVALID_SOCKET;
		u_short						port;
		ULONG						ip;
		u_int64						sessionID;

		Session()
			: isSending(false)
			, sessionID(0)
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
		{
			ZeroMemory(&recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};

	class CLF_NetServer
	{
	protected:
		CLF_NetServer();
		virtual ~CLF_NetServer();
		bool Start();
		void Stop();

		bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
		void SendPacket(SESSION_ID SessionID, CNetPacket* packet); // SESSION_ID / HOST_ID
		void SendPacketToWorker(SESSION_ID SessionID, CNetPacket* packet);
		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후

		virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept 후 접속처리 완료 후 호출.
		virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release 후 호출
		void LoadInitFile(const WCHAR* fileName);

		virtual void OnRecv(SESSION_ID SessionID, CNetPacket* packet) = 0; //< 패킷 수신 완료 후
		//	virtual void OnSend(SessionID, int sendsize) = 0;           < 패킷 송신 완료 후

		//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
		//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

		virtual void OnError(int errorcode, const WCHAR* log) = 0;
		void QuitServer();
		void SetZeroCopy(bool on);
		void SetNagle(bool on);
		ULONG GetSessionIP(SESSION_ID sessionNo);

	private:
		void	Init();
		Session* FindSession(SESSION_ID sessionNo);
		void InsertSessionData(Session* session);
		void DeleteSessionData(SESSION_ID sessionNo);
		void UpdateSessionData(SESSION_ID sessionNo, Session* session);
		bool CreateIOCP();
		bool CreateListenSocket();
		bool BeginThreads();
		static unsigned int WINAPI WorkerThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
		static unsigned int WINAPI MonitorThread(LPVOID arg);
		void MonitorProc();
		bool RecvPost(Session* session, bool isAccepted = false);
		bool SendPost(Session* session);
		void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv);
		void IncrementIOProc(Session* session, int logic);
		void DecrementIOProc(Session* session, int logic);
		void ReleaseProc(Session* session);
		void AcceptProc();
		Session* CreateSession(SOCKET client, SOCKADDR_IN clientAddr);
		void GQCS();
		void CompleteRecv(Session* session, DWORD transferredSize);
		void CompleteSend(Session* session, DWORD transferredSize);
		void CloseSessions();
		void InitializeEmptyIndex();
		SESSION_ID GenerateSessionID();
		u_short GetIndexFromSessionNo(SESSION_ID sessionNo);
		u_int64 GetLowNumFromSessionNo(SESSION_ID sessionNo);

	private:
		enum {
			SEND_BUF_SIZE = 8192
		};

		SOCKET				mListenSocket = INVALID_SOCKET;
		BYTE				mNumThreads = 0;
		HANDLE				mHcp = INVALID_HANDLE_VALUE;
		HANDLE*				mhThreads = nullptr;
		Session*			mSessionArray = nullptr;
		u_int64				mSessionIDCounter = 1;
		TC_LFStack<u_short> mEmptyIndexes;
		u_short				mPort = 0;
		WCHAR				mBindIP[32];
		u_short				mMaxClient = 0;
		HANDLE				mBeginEvent = INVALID_HANDLE_VALUE;

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
		bool				mbZeroCopy = false;
		bool				mbExit = false;
		bool				mbBegin = false;
		BYTE				mActiveThreadNum = 0;
		BYTE				mWorkerThreadNum = 0;

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
