#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include "RingBuffer.h"
#include "TC_LFQueue.h"

namespace procademy
{
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
	};

	class CLanClient
	{
	protected:
		CLanClient();
		virtual ~CLanClient();

		bool Connect(const WCHAR* serverIP, USHORT serverPort);	//바인딩 IP, 서버IP / 워커스레드 수 / 나글옵션
		bool Disconnect();
		bool SendPacket(CNetPacket* packet);

		virtual void OnEnterJoinServer() = 0; //< 서버와의 연결 성공 후
		virtual void OnLeaveServer() = 0; //< 서버와의 연결이 끊어졌을 때

		virtual void OnRecv(CNetPacket*) = 0; //< 하나의 패킷 수신 완료 후
		virtual void OnSend(int sendsize) = 0; //< 패킷 송신 완료 후

		//	virtual void OnWorkerThreadBegin() = 0;
		//	virtual void OnWorkerThreadEnd() = 0;

		virtual void OnError(int errorcode, const WCHAR*) = 0;
		void	SetZeroCopy(bool on);
		void	SetNagle(bool on);

	private:
		static unsigned int WINAPI WorkerThread(LPVOID arg);
		static unsigned int WINAPI MonitorThread(LPVOID arg);

		void	Init();
		void	LoadInitFile(const WCHAR* fileName);
		bool	CreateIOCP();
		bool	BeginThreads();
		bool	CreateSocket();
		void	GQCS();
		void	CompleteRecv(DWORD transferredSize);
		void	CompleteSend(DWORD transferredSize);
		void	MonitorProc();
		bool	RecvPost(bool isFirst = false);
		bool	SendPost();
		void	SetWSABuf(WSABUF* bufs, bool isRecv);
		void	IncrementIOProc(int logic);
		void	DecrementIOProc(int logic);
		void	ReleaseProc();


	private:
		enum {
			SEND_BUF_SIZE = 8192
		};

		HANDLE				mIocp = INVALID_HANDLE_VALUE;
		HANDLE*				mhThreads = nullptr;
		BYTE				mNumThreads = 0;
		Session				mClient;
		USHORT				mServerPort = 0;
		WCHAR				mServerIP[32];

		DWORD				sendTPS;
		DWORD				recvTPS;

	protected:
		bool				mbNagle = true;
		bool				mbZeroCopy = true;
		bool				mbExit = false;
		bool				mbBegin = false;
		BYTE				mActiveThreadNum = 0;
		BYTE				mWorkerThreadNum = 0;

		struct Monitor
		{
			DWORD			prevSendTPS = 0;
			DWORD			prevRecvTPS = 0;
		};

		Monitor		mMonitor;
	};
}
