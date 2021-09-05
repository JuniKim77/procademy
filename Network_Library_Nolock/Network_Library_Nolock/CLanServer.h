#pragma once
#pragma comment(lib, "ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "RingBuffer.h"
#include "ObjectPool.h"
#include <unordered_map>

typedef u_int64 SESSION_ID;

class CPacket;

class CLanServer
{
private:
	struct OverlappedBuffer
	{
		WSAOVERLAPPED overlapped;
		bool type;
		RingBuffer queue;
	};

	struct Session
	{
		OverlappedBuffer recv;
		OverlappedBuffer send;
		SOCKET socket;
		char ioCount;
		bool isSending;
		u_short port;
		ULONG ip;
		u_int64 sessionID;
		SRWLOCK lock;

		Session()
			: ioCount(0)
			, isSending(false)
			, sessionID(0)
		{
			ZeroMemory(&recv.overlapped, sizeof(recv.overlapped));
			ZeroMemory(&send.overlapped, sizeof(send.overlapped));
			recv.type = true;
			send.type = false;
			InitializeSRWLock(&lock);
		}

		Session(SOCKET _socket, ULONG _ip, u_short _port)
			: socket(_socket)
			, ip(_ip)
			, port(_port)
			, ioCount(0)
			, isSending(false)
			, sessionID(0)
		{
			ZeroMemory(&recv.overlapped, sizeof(recv.overlapped));
			ZeroMemory(&send.overlapped, sizeof(send.overlapped));
			recv.type = true;
			send.type = false;
			InitializeSRWLock(&lock);
		}
	};

public:
	~CLanServer();
	bool Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
	bool Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);
	void Stop();
	int GetSessionCount();
	void WaitForThreadsFin();

	bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
	bool SendPacket(SESSION_ID SessionID, CPacket* packet); // SESSION_ID / HOST_ID

	virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후

	virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release 후 호출


	virtual void OnRecv(SESSION_ID SessionID, CPacket* packet) = 0; //< 패킷 수신 완료 후
	//	virtual void OnSend(SessionID, int sendsize) = 0;           < 패킷 송신 완료 후

	//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
	//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

	virtual void OnError(int errorcode, WCHAR* log) = 0;

private:
	Session* FindSession(u_int64 sessionNo);
	void InsertSessionData(u_int64 sessionNo, Session* session);
	void DeleteSessionData(u_int64 sessionNo);
	void UpdateSessionData(u_int64 sessionNo, Session* session);
	bool CreateListenSocket();
	bool BeginThreads();
	static unsigned int WINAPI WorkerThread(LPVOID arg);
	static unsigned int WINAPI AcceptThread(LPVOID arg);
	bool RecvPost(Session* session);
	bool SendPost(Session* session);
	void SetWSABuf(WSABUF* bufs, Session* session, bool isRecv);
	bool DecrementProc(Session* session);
	void DisconnectProc(Session* session);
	void PacketProc(Session* session, DWORD msgSize);
	bool AcceptProc();
	Session* CreateSession(SOCKET client, SOCKADDR_IN clientAddr);
	bool OnCompleteMessage();
	void CloseSessions();

private:

};
