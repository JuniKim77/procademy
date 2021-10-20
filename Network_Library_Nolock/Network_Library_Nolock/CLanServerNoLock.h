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
	class CPacket;

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
		TC_LFQueue<CPacket*>		sendQ;
		int							numSendingPacket = 0;
		alignas(64) SessionIoCount	ioBlock;
		alignas(64) bool			isSending;
		bool						bIsAlive;
		SOCKET						socket = INVALID_SOCKET;
		u_short						port;
		ULONG						ip;
		u_int64						sessionID;
		DWORD						lastNum = 0;

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
	public:
		~CLanServerNoLock();
		bool Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // ���� IP / ��Ʈ / ��Ŀ������ ��(������, ���׼�) / ���ۿɼ� / �ִ������� ��
		bool Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);
		void Stop();
		int GetSessionCount();
		void WaitForThreadsFin();

		bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
		void SendPacket(SESSION_ID SessionID, CPacket* packet); // SESSION_ID / HOST_ID

		virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept ����

		virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept �� ����ó�� �Ϸ� �� ȣ��.
		virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release �� ȣ��


		virtual void OnRecv(SESSION_ID SessionID, CPacket* packet) = 0; //< ��Ŷ ���� �Ϸ� ��
		//	virtual void OnSend(SessionID, int sendsize) = 0;           < ��Ŷ �۽� �Ϸ� ��

		//	virtual void OnWorkerThreadBegin() = 0;                    < ��Ŀ������ GQCS �ٷ� �ϴܿ��� ȣ��
		//	virtual void OnWorkerThreadEnd() = 0;                      < ��Ŀ������ 1���� ���� ��

		virtual void OnError(int errorcode, const WCHAR* log) = 0;
		void MonitorProc();

	private:
		Session* FindSession(u_int64 sessionNo);
		void InsertSessionData(Session* session);
		void DeleteSessionData(u_int64 sessionNo);
		void UpdateSessionData(u_int64 sessionNo, Session* session);
		bool CreateListenSocket();
		bool BeginThreads();
		static unsigned int WINAPI WorkerThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
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
		bool				mbNagle = true;
		bool				mbMonitoring = true;
		BYTE				mMaxRunThreadSize = 0;
		BYTE				mWorkerThreadSize = 0;
		u_short				mMaxClient = 0;

		/// <summary>
		/// Network Status
		/// </summary>
		bool				mbIsRunning = false;
		bool				mbZeroCopy = true;
		BYTE				mNumThreads = 0;

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

		struct Monitor
		{
			alignas(64) DWORD	sendTPS;
			alignas(64) DWORD	recvTPS;
			alignas(64) DWORD	acceptCount;
			alignas(64) DWORD	disconnectCount;
			SRWLOCK				lock;
		};

		/// <summary>
		/// Monitoring members
		/// </summary>
		alignas(64) Monitor		mMonitor;
	};
}

