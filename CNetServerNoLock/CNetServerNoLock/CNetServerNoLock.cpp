#pragma warning(disable:6387)

#include "CNetServerNoLock.h"
#include "CLogger.h"
#include "CNetPacket.h"
#include "CCrashDump.h"
#include "CProfiler.h"
#include "TextParser.h"
#include <timeapi.h>

#pragma comment(lib, "winmm")

struct sessionDebug;

extern sessionDebug g_sessionLog[USHRT_MAX + 1];
extern USHORT g_sessionIdx;

extern void _sessionLog(
	UINT64 playerNo,
	UINT64 sessionNo,
	DWORD lastTime,
	DWORD threadId,
	int type,
	int loginID);

namespace procademy
{
	void CLF_NetServer::Init()
	{
		CLogger::SetDirectory(L"_log");

		mBeginEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);

		mhThreads = new HANDLE[(long long)mWorkerThreadNum + 2];
		mSessionArray = (Session*)_aligned_malloc(sizeof(Session) * mMaxClient, 64);
		for (int i = 0; i < mMaxClient; ++i)
		{
			new (&mSessionArray[i]) (Session);
		}

		InitializeEmptyIndex();
		CreateIOCP();
	}

	CLF_NetServer::Session* CLF_NetServer::FindSession(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray + index;
	}

	void CLF_NetServer::InsertSessionData(Session* session)
	{
		u_short index = GetIndexFromSessionNo(session->sessionID);

		mSessionArray[index] = *session;
	}

	void CLF_NetServer::DeleteSessionData(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		mEmptyIndexes.Push(index);
	}

	void CLF_NetServer::UpdateSessionData(SESSION_ID sessionNo, Session* session)
	{
	}

	bool CLF_NetServer::CreateIOCP()
	{
		// 논리 코어 개수 확인 로직
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		if (mActiveThreadNum > si.dwNumberOfProcessors)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Setting: Max Running thread is larger than the number of processors");
		}

		mHcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, mActiveThreadNum);

		if (mHcp == NULL)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());

			return false;
		}

		return true;
	}

	bool CLF_NetServer::CreateListenSocket()
	{
		WSADATA			wsa;
		SOCKADDR_IN		addr;
		LINGER			optval;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup [Error: %d]", WSAGetLastError());
			return false;
		}

		mListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (mListenSocket == INVALID_SOCKET)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create socket [Error: %d]", WSAGetLastError());
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Create");

		addr.sin_family = AF_INET;
		addr.sin_port = htons(mPort);
		InetPton(AF_INET, mBindIP, &addr.sin_addr);

		int bindRet = bind(mListenSocket, (SOCKADDR*)&addr, sizeof(addr));

		if (bindRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Bind [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Bind");

		// 백로그 길이
		int listenRet = listen(mListenSocket, SOMAXCONN_HINT(mMaxClient));

		if (listenRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Listen [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Set Listening");

		// TimeWait Zero
		optval.l_onoff = 1;
		optval.l_linger = 0;

		int timeOutnRet = setsockopt(mListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));
		if (timeOutnRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Listen Socket Linger [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Option : TimeOut Zero");

		SetZeroCopy(mbZeroCopy);
		SetNagle(mbNagle);

		return true;
	}

	bool CLF_NetServer::BeginThreads()
	{
		BYTE i = 0;

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);

		for (; i <= mWorkerThreadNum; ++i)
		{
			mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
		}

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
		
		mNumThreads = i;

		return true;
	}

	unsigned int __stdcall CLF_NetServer::WorkerThread(LPVOID arg)
	{
		CLF_NetServer* server = (CLF_NetServer*)arg;

		while (!server->mbExit)
		{
			server->GQCS();
		}

		return 0;
	}

	unsigned int __stdcall CLF_NetServer::AcceptThread(LPVOID arg)
	{
		CLF_NetServer* server = (CLF_NetServer*)arg;

		while (!server->mbExit)
		{
			if (server->mbBegin)
			{
				server->AcceptProc();
			}
			else
			{
				WaitForSingleObject(server->mBeginEvent, INFINITE);
				//server->mbBegin = true;
			}
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServer Accept Thread End");

		return 0;
	}

	unsigned int __stdcall CLF_NetServer::MonitorThread(LPVOID arg)
	{
		CLF_NetServer* server = (CLF_NetServer*)arg;

		server->MonitorProc();

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServer Monitor Thread End");

		return 0;
	}

	bool CLF_NetServer::RecvPost(Session* session, bool isAccepted)
	{
		if (!isAccepted)
		{
			IncrementIOProc(session, 30000);
		}

		WSABUF buffers[2];
		DWORD flags = 0;

		SetWSABuf(buffers, session, true);

		int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recvOverlapped, nullptr);

		if (recvRet == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err == WSA_IO_PENDING)
			{
				return true;
			}

			if (err != WSAECONNRESET)
			{
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"RecvPost Unusual [Error: %d]", err);
			}

			DecrementIOProc(session, 10050);

			return false;
		}

		return true;
	}

	bool CLF_NetServer::SendPost(Session* session)
	{
		WSABUF buffers[100];
		bool ret = true;

		do
		{
			if (InterlockedExchange8((char*)&session->isSending, (char)true) == (char)true)
			{
				break;
			}

			if (session->sendQ.IsEmpty() == true)
			{
				session->isSending = false;

				if (session->sendQ.IsEmpty() == false)
				{
					continue;
				}

				break;
			}

			SetWSABuf(buffers, session, false);

			ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));

			IncrementIOProc(session, 30000);

			int sendRet = WSASend(session->socket, buffers, session->numSendingPacket, nullptr, 0, &session->sendOverlapped, nullptr);

			if (sendRet == SOCKET_ERROR)
			{
				int err = WSAGetLastError();

				if (err == WSA_IO_PENDING)
				{
					break;
				}

				if (err != WSAECONNRESET)
				{
					CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendPost Unusual [Error: %d]", err);
				}

				DecrementIOProc(session, 20050);

				ret = false;
			}

			break;
		} while (1);

		return ret;
	}

	void CLF_NetServer::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
	{	
		if (isRecv)
		{
			char* pRear = session->recvQ.GetRearBufferPtr();
			char* pFront = session->recvQ.GetFrontBufferPtr();
			char* pBuf = session->recvQ.GetBuffer();
			char* pEnd = session->recvQ.GetEndBuffer();

			if (pRear < pFront)
			{
				bufs[0].buf = pRear;
				bufs[0].len = (ULONG)(pFront - pRear - 1);
				bufs[1].buf = pRear;
				bufs[1].len = 0;
			}
			else
			{
				if (pFront == pBuf)
				{
					bufs[0].buf = pRear;
					bufs[0].len = (ULONG)(pEnd - pRear - 1);
					bufs[1].buf = pBuf;
					bufs[1].len = 0;
				}
				else
				{
					bufs[0].buf = pRear;
					bufs[0].len = (ULONG)(pEnd - pRear);
					bufs[1].buf = pBuf;
					bufs[1].len = (ULONG)(pFront - pBuf - 1);
				}
			}
		}
		else
		{
			CNetPacket* packetBufs[100];
			DWORD snapSize = session->sendQ.Peek(packetBufs, 100);

			for (DWORD i = 0; i < snapSize; ++i)
			{
				bufs[i].buf = packetBufs[i]->GetZeroPtr();
				bufs[i].len = packetBufs[i]->GetSize();
			}

			session->numSendingPacket = snapSize;
		}
	}

	void CLF_NetServer::IncrementIOProc(Session* session, int logic)
	{
		InterlockedIncrement(&session->ioBlock.ioCount);
		/*ioDebugLog(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
	}

	void CLF_NetServer::DecrementIOProc(Session* session, int logic)
	{
		SessionIoCount ret;

		ret.ioCount = InterlockedDecrement(&session->ioBlock.ioCount);

		if (ret.releaseCount.count < 0)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"[SessionID: %llu] [ioCount: %d] [isSending: %d] [recv: %d] [Send: %d]",
				session->sessionID & 0xffffffffff,
				session->ioBlock.ioCount,
				session->isSending,
				session->recvQ.GetUseSize(),
				session->sendQ.GetSize());

			CRASH();
		}

		if (ret.ioCount == 0)
		{
			ReleaseProc(session);
		}
	}

	void CLF_NetServer::ReleaseProc(Session* session)
	{
		SessionIoCount released;
		CNetPacket* dummy;

		released.ioCount = 0;
		released.releaseCount.isReleased = 1;

		if (InterlockedCompareExchange(&session->ioBlock.ioCount, released.ioCount, 0) != 0)
		{
			return;
		}
		//
		u_int64 id = session->sessionID;
		session->sessionID = 0;

		OnClientLeave(id);

		closesocket(session->socket);

		session->isSending = false;

		while (1)
		{
			if (session->sendQ.Dequeue(&dummy) == false)
			{
				break;
			}
			
			dummy->SubRef();
		}

		session->recvQ.ClearBuffer();

		ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));

		DeleteSessionData(id);
	}

	void CLF_NetServer::AcceptProc()
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET client = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (client == INVALID_SOCKET)
		{
			int err = WSAGetLastError();

			if (err == WSAENOTSOCK || err == WSAEINTR)
			{
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"Listen Socket Close [Error: %d]", err);

				return;
			}

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Listen Socket [Error: %d]", err);

			return;
		}

		if (OnConnectionRequest(clientAddr.sin_addr.S_un.S_addr, clientAddr.sin_port) == false)
		{
			WCHAR IP[16] = { 0, };

			InetNtop(AF_INET, &clientAddr.sin_addr.S_un.S_addr, IP, 16);

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept Denied [IP: %s] [Port: %u]",
				IP, ntohs(clientAddr.sin_port));

			closesocket(client);

			return;
		}

		Session* session = CreateSession(client, clientAddr);

		if (session == nullptr)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create Session Fail");
			return;
		}

		//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Socket Accept [IP: %s] [Port: %u]",
		//    IP, ntohs(clientAddr.sin_port));

		IncrementIOProc(session, 30000);
		session->ioBlock.releaseCount.isReleased = 0;

		/*ioDebug(10020, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		OnClientJoin(session->sessionID);
#ifdef PROFILE
		CProfiler::Begin(L"RecvPost");
#endif // PROFILE
		RecvPost(session, true);
#ifdef PROFILE
		CProfiler::End(L"RecvPost");
#endif // PROFILE
	}

	CLF_NetServer::Session* CLF_NetServer::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
	{
		u_int64 id = GenerateSessionID();

		if (id == 0)
		{
			return nullptr;
		}

		Session* session = FindSession(id);
		session->socket = client;
		session->ip = clientAddr.sin_addr.S_un.S_addr;
		session->port = clientAddr.sin_port;
		session->sessionID = id;

		HANDLE hResult = CreateIoCompletionPort((HANDLE)client, mHcp, (ULONG_PTR)session, 0);

		if (hResult == NULL)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);

			return nullptr;
		}

		//InterlockedIncrement((LONG*)&mMonitor.acceptCount);
		acceptTotal++;
		acceptTPS++;

		return session;
	}

	void CLF_NetServer::GQCS()
	{
		while (1)
		{
			DWORD transferredSize = 0;
			Session* completionKey = nullptr;
			WSAOVERLAPPED* pOverlapped = nullptr;
			Session* session = nullptr;
			int err = 0;
			
			BOOL gqcsRet = GetQueuedCompletionStatus(mHcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);
#ifdef PROFILE
			CProfiler::Begin(L"GQCS_Net");
#endif // PROFILE
			session = (Session*)completionKey;

			if (transferredSize == 0)
			{
				INT64 type = (INT64)pOverlapped;

				switch (type)
				{
				case 0: // ECHO Server End
					PostQueuedCompletionStatus(mHcp, 0, 0, 0);
					return;
				case 1: // SEND_TO_WORKER
#ifdef PROFILE
					CProfiler::Begin(L"SendPost");
					SendPost(session);
					CProfiler::End(L"SendPost");
#else
					SendPost(session);
#endif // PROFILE
					break;
				case 2: // DISCONNECT
					DisconnectProc((SESSION_ID)completionKey);
					continue;
				default:
					err = WSAGetLastError();

					if (err == WSA_OPERATION_ABORTED || err == ERROR_CONNECTION_ABORTED)
					{
						_sessionLog(10, session->sessionID, 10, GetCurrentThreadId(), 1, 30000);
						CLogger::_Log(dfLOG_LEVEL_ERROR, L"Disconnect [Session ID: %llu] %d", session->sessionID, err);
					}
					break;
				}
			}
			else
			{
				if (pOverlapped == &session->recvOverlapped) // Recv
				{
#ifdef PROFILE
					CProfiler::Begin(L"CompleteRecv");
					CompleteRecv(session, transferredSize);
					CProfiler::End(L"CompleteRecv");
#else
					CompleteRecv(session, transferredSize);
#endif // PROFILE
				}
				else // Send
				{
#ifdef PROFILE
					CProfiler::Begin(L"CompleteSend");
					CompleteSend(session, transferredSize);
					CProfiler::End(L"CompleteSend");
#else
					CompleteSend(session, transferredSize);
#endif // PROFILE
				}
			}

			DecrementIOProc(session, 10000);
#ifdef PROFILE
			CProfiler::End(L"GQCS_Net");
#endif // PROFILE
		}
	}

	void CLF_NetServer::CompleteRecv(Session* session, DWORD transferredSize)
	{
		session->recvQ.MoveRear(transferredSize);
		CNetPacket::st_Header header;
		bool status = true;

		while (1)
		{
			int useSize = session->recvQ.GetUseSize();

			if (useSize <= CNetPacket::HEADER_MAX_SIZE)
				break;

			session->recvQ.Peek((char*)&header, CNetPacket::HEADER_MAX_SIZE);

			if (header.code != CNetPacket::sCode)
			{
				status = false;

				//CLogger::_Log(dfLOG_LEVEL_ERROR, L"Session ID: %d - Code Error", session->sessionID);
				break;
			}

			if (header.len > CNetPacket::eBUFFER_DEFAULT)
			{
				status = false;

				//CLogger::_Log(dfLOG_LEVEL_ERROR, L"Session ID: %d - Header Len Error", session->sessionID);
				break;
			}

			if (useSize < (CNetPacket::HEADER_MAX_SIZE + header.len))
				break;

			CNetPacket* packet = CNetPacket::AllocAddRef();

			memcpy_s(packet->GetZeroPtr(), CNetPacket::HEADER_MAX_SIZE, (char*)&header, CNetPacket::HEADER_MAX_SIZE);

			InterlockedIncrement(&recvTPS);

			session->recvQ.MoveFront(CNetPacket::HEADER_MAX_SIZE);

			int ret = session->recvQ.Dequeue(packet->GetFrontPtr(), (int)header.len);

			packet->MoveRear(ret);
#ifdef PROFILE
			CProfiler::Begin(L"Decode");
#endif			
			if (packet->Decode() == false)
			{
				status = false;
				packet->SubRef();
				break;
			}
#ifdef PROFILE
			CProfiler::End(L"Decode");
#endif			
			OnRecv(session->sessionID, packet); // -> SendPacket

			packet->SubRef();
		}
#ifdef PROFILE
		CProfiler::Begin(L"RecvPost");
		if (status)
		{
			RecvPost(session);
		}
		CProfiler::End(L"RecvPost");
#else
		if (status)
		{
			RecvPost(session);
		}
#endif // PROFILE
	}

	void CLF_NetServer::CompleteSend(Session* session, DWORD transferredSize)
	{
		CNetPacket* packet;
		InterlockedAdd((LONG*)&sendTPS, session->numSendingPacket);
		for (int i = 0; i < session->numSendingPacket; ++i)
		{
			session->sendQ.Dequeue(&packet);

			packet->SubRef();
			packet = nullptr;
		}

		session->numSendingPacket = 0;
		session->isSending = false;
#ifdef PROFILE
		CProfiler::Begin(L"SendPost");
		SendPost(session);
		CProfiler::End(L"SendPost");
#else
		SendPost(session);
#endif // PROFILE		
	}

	void CLF_NetServer::CloseSessions()
	{
	}

	void CLF_NetServer::InitializeEmptyIndex()
	{
		for (u_short i = mMaxClient; i > 0; --i)
		{
			mEmptyIndexes.Push(i - 1);
		}

		//InitializeSRWLock(&mStackLock);
	}

	SESSION_ID CLF_NetServer::GenerateSessionID()
	{
		if (mEmptyIndexes.IsEmpty())
		{
			return 0;
		}

		u_short index = 0;
		mEmptyIndexes.Pop(&index);
		u_int64 id = index;

		id <<= (8 * 6);

		id |= mSessionIDCounter;
		mSessionIDCounter++;

		return id;
	}

	u_short CLF_NetServer::GetIndexFromSessionNo(SESSION_ID sessionNo)
	{
		return (u_short)(sessionNo >> (8 * 6));
	}

	u_int64 CLF_NetServer::GetLowNumFromSessionNo(SESSION_ID sessionNo)
	{
		return sessionNo & 0xffffffffffff;
	}

	void CLF_NetServer::DisconnectProc(SESSION_ID SessionID)
	{
		Session* session = FindSession(SessionID);
		BOOL ret;

		IncrementIOProc(session, 40000);

		if (session->ioBlock.releaseCount.isReleased == 1 || SessionID != session->sessionID)
		{
			_sessionLog(SessionID, session->sessionID, 10, GetCurrentThreadId(), 1, 10000);
			DecrementIOProc(session, 40020);
			return;
		}

		_sessionLog(SessionID, session->sessionID, 10, GetCurrentThreadId(), 1, 10010);
		ret = CancelIoEx((HANDLE)session->socket, nullptr);
		_sessionLog(SessionID, session->sessionID, 10, GetCurrentThreadId(), 1, 10020);
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Disconnect [ReqSessionNo: %llu] [SessionID: %llu][io:%d][rel:%d]",
			SessionID, session->sessionID, session->ioBlock.ioCount, session->ioBlock.releaseCount.isReleased);

		DecrementIOProc(session, 40040);
	}

	ULONG CLF_NetServer::GetSessionIP(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray[index].ip;
	}

	void CLF_NetServer::SetZeroCopy(bool on)
	{
		int optNum = on ? 0 : SEND_BUF_SIZE;

		if (setsockopt(mListenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);

			return;
		}

		if (on)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Zero Copy On");
		}
		else
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Zero Copy Off");
		}
	}

	void CLF_NetServer::SetNagle(bool on)
	{
		BOOL optval = on;

		if (setsockopt(mListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]", WSAGetLastError());
			closesocket(mListenSocket);

			return;
		}

		if (on)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Nagle On");
		}
		else
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Nagle Off");
		}
	}

	void CLF_NetServer::MonitorProc()
	{
		HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

		while (!mbExit)
		{
			DWORD retval = WaitForSingleObject(dummyEvent, 1000);

			if (retval == WAIT_TIMEOUT)
			{
				mMonitor.prevRecvTPS = recvTPS;
				mMonitor.prevSendTPS = sendTPS;
				mMonitor.acceptTotal = acceptTotal;
				mMonitor.acceptTPS = acceptTPS;

				recvTPS = 0;
				sendTPS = 0;
				acceptTPS = 0;
			}
		}

		CloseHandle(dummyEvent);
	}

	void CLF_NetServer::QuitServer()
	{
		mbExit = true;

		Stop();

		PostQueuedCompletionStatus(mHcp, 0, 0, 0);

		SetEvent(mBeginEvent);

		DWORD waitResult = WaitForMultipleObjects(mNumThreads, mhThreads, TRUE, INFINITE);

		switch (waitResult)
		{
		case WAIT_FAILED:
			wprintf_s(L"Main Thread Handle Error");
			break;
		case WAIT_TIMEOUT:
			wprintf_s(L"Main Thread Timeout Error");
			break;
		case WAIT_OBJECT_0:
			wprintf_s(L"None Error\n");
			break;
		default:
			break;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Quit CNetServer");
	}

	CLF_NetServer::CLF_NetServer()
	{
		timeBeginPeriod(1);

		WSADATA		wsa;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup [Error: %d]", WSAGetLastError());
		}
	}

	CLF_NetServer::~CLF_NetServer()
	{
		closesocket(mListenSocket);
		if (mhThreads != nullptr)
		{
			delete[] mhThreads;
		}
		if (mSessionArray != nullptr)
		{
			_aligned_free(mSessionArray);
		}
		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServerNoLock End");

		timeEndPeriod(1);
	}

	bool CLF_NetServer::Start()
	{
		if (mbBegin == true)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running");
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServer Begin");

		if (!CreateListenSocket())
		{
			return false;
		}

		mbBegin = true;

		SetEvent(mBeginEvent);

		return true;
	}

	void CLF_NetServer::Stop()
	{
		mbBegin = false;
		BOOL ret;

		closesocket(mListenSocket);

		for (USHORT i = 0; i < mMaxClient; ++i)
		{
			ret = CancelIoEx((HANDLE)mSessionArray[i].socket, nullptr);
			//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Cancel Ret: %d, Err: %d", ret, GetLastError());
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Stop CNetServer");
	}

	void CLF_NetServer::Begin()
	{
		Init();
		BeginThreads();
	}

	void CLF_NetServer::SetServerIP(const WCHAR* server)
	{
		wcscpy_s(mBindIP, _countof(mBindIP), server);
	}

	void CLF_NetServer::SetServerPort(USHORT port)
	{
		mPort = port;
	}

	void CLF_NetServer::Disconnect(SESSION_ID SessionID)
	{
		PostQueuedCompletionStatus(mHcp, 0, (ULONG_PTR)SessionID, (LPOVERLAPPED)2);
	}

	void CLF_NetServer::SendPacket(SESSION_ID SessionID, CNetPacket* packet)
	{
		Session* session = FindSession(SessionID);
		//
		IncrementIOProc(session, 20000);

		if (session->ioBlock.releaseCount.isReleased == 1 || SessionID != session->sessionID)
		{
			DecrementIOProc(session, 20020);
			/*USHORT ret = InterlockedIncrement16((SHORT*)&g_debugPacket);
			g_sessionDebugs[ret] = packet;*/
			return;
		}
		
		packet->ReadySend();
		packet->AddRef();
		session->sendQ.Enqueue(packet);

#ifdef PROFILE
		CProfiler::Begin(L"SendPost");
		SendPost(session);
		CProfiler::End(L"SendPost");
#else
		SendPost(session);
#endif // PROFILE

		DecrementIOProc(session, 20020);
	}

	void CLF_NetServer::SendPacketToWorker(SESSION_ID SessionID, CNetPacket* packet)
	{
		Session* session = FindSession(SessionID);
		//
		IncrementIOProc(session, 20000);

		if (session->ioBlock.releaseCount.isReleased == 1 || SessionID != session->sessionID)
		{
			DecrementIOProc(session, 20020);
			/*USHORT ret = InterlockedIncrement16((SHORT*)&g_debugPacket);
			g_sessionDebugs[ret] = packet;*/
			return;
		}

		packet->ReadySend();
		packet->AddRef();
		session->sendQ.Enqueue(packet);

		if (session->isSending == false)
		{
			IncrementIOProc(session, 20010);
			PostQueuedCompletionStatus(mHcp, 0, (ULONG_PTR)session, (LPOVERLAPPED)1);
		}

		DecrementIOProc(session, 20020);
	}
}
