#include "CLF_LanServer.h"
#include "CLogger.h"
#include "CLanPacket.h"
#include "CCrashDump.h"
#include "CProfiler.h"
#include "TextParser.h"

namespace procademy
{
	void CLF_LanServer::Init()
	{
		WORD		version = MAKEWORD(2, 2);
		WSADATA		data;

		WSAStartup(version, &data);
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

	CLF_LanServer::Session* CLF_LanServer::FindSession(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray + index;
	}

	void CLF_LanServer::InsertSessionData(Session* session)
	{
		u_short index = GetIndexFromSessionNo(session->sessionID);

		mSessionArray[index] = *session;
	}

	void CLF_LanServer::DeleteSessionData(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		mEmptyIndexes.Push(index);
	}

	void CLF_LanServer::UpdateSessionData(SESSION_ID sessionNo, Session* session)
	{
	}

	bool CLF_LanServer::CreateIOCP()
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
			closesocket(mListenSocket);
			return false;
		}

		return true;
	}

	bool CLF_LanServer::CreateListenSocket()
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
		int listenRet = listen(mListenSocket, mMaxClient);

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

	bool CLF_LanServer::BeginThreads()
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

	unsigned int __stdcall CLF_LanServer::WorkerThread(LPVOID arg)
	{
		CLF_LanServer* server = (CLF_LanServer*)arg;

		while (!server->mbExit)
		{
			server->GQCS();
		}

		return 0;
	}

	unsigned int __stdcall CLF_LanServer::AcceptThread(LPVOID arg)
	{
		CLF_LanServer* server = (CLF_LanServer*)arg;

		while (!server->mbExit)
		{
			if (server->mbBegin)
			{
				server->AcceptProc();
			}
			else
			{
				WaitForSingleObject(server->mBeginEvent, INFINITE);
				server->mbBegin = true;
			}
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CLanServer Accept Thread End");

		return 0;
	}

	unsigned int __stdcall CLF_LanServer::MonitorThread(LPVOID arg)
	{
		CLF_LanServer* server = (CLF_LanServer*)arg;

		server->MonitorProc();

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CLanServer Monitor Thread End");

		return 0;
	}

	bool CLF_LanServer::RecvPost(Session* session, bool isAccepted)
	{
		if (!isAccepted)
		{
			IncrementIOProc(session, 30000);
		}

		WSABUF buffers[2];
		DWORD flags = 0;

		SetWSABuf(buffers, session, true);

#ifdef PROFILE
		CProfiler::Begin(L"WSARecv");
#endif // PROFILE
		int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recvOverlapped, nullptr);

		if (recvRet == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err == WSA_IO_PENDING)
			{
				return true;
			}

			DecrementIOProc(session, 10050);

			return false;
		}

		return true;
	}

	bool CLF_LanServer::SendPost(Session* session)
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

#ifdef PROFILE
			CProfiler::Begin(L"WSASend");
#endif // PROFILE
			int sendRet = WSASend(session->socket, buffers, session->numSendingPacket, nullptr, 0, &session->sendOverlapped, nullptr);

			if (sendRet == SOCKET_ERROR)
			{
				int err = WSAGetLastError();

				if (err == WSA_IO_PENDING)
				{
					break;
				}

				DecrementIOProc(session, 20050);

				ret = false;
			}

			break;
		} while (1);

		return ret;
	}

	void CLF_LanServer::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
	{
#ifdef PROFILE
		CProfiler::Begin(L"SetWSABuf");
#endif // PROFILE

		if (isRecv)
		{
			char* pRear = session->recvQ.GetRearBufferPtr();
			char* pFront = session->recvQ.GetFrontBufferPtr();
			char* pBuf = session->recvQ.GetBuffer();
			int capa = session->recvQ.GetCapacity();

			if (pRear < pFront)
			{
				bufs[0].buf = pRear;
				bufs[0].len = (ULONG)(pRear - pFront);
				bufs[1].buf = pRear;
				bufs[1].len = 0;
			}
			else
			{
				bufs[0].buf = pRear;
				bufs[0].len = (ULONG)(capa + 1 - (pRear - pBuf));
				bufs[1].buf = pBuf;
				bufs[1].len = (ULONG)(pFront - pBuf);
			}
		}
		else
		{
			CLanPacket* packetBufs[100];
			DWORD snapSize = session->sendQ.Peek(packetBufs, 100);

			for (DWORD i = 0; i < snapSize; ++i)
			{
				bufs[i].buf = packetBufs[i]->GetZeroPtr();
				bufs[i].len = packetBufs[i]->GetSize();
			}

			session->numSendingPacket = snapSize;
		}
#ifdef PROFILE
		CProfiler::End(L"SetWSABuf");
#endif // PROFILE
	}

	void CLF_LanServer::IncrementIOProc(Session* session, int logic)
	{
		InterlockedIncrement(&session->ioBlock.ioCount);
		/*ioDebugLog(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
	}

	void CLF_LanServer::DecrementIOProc(Session* session, int logic)
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

	void CLF_LanServer::ReleaseProc(Session* session)
	{
		SessionIoCount released;
		CLanPacket* dummy;

		released.ioCount = 0;
		released.releaseCount.isReleased = 1;

		if (InterlockedCompareExchange(&session->ioBlock.ioCount, released.ioCount, 0) != 0)
		{
			return;
		}
		//
		u_int64 id = session->sessionID;
		session->sessionID = 0;
		InterlockedIncrement((LONG*)&disconnectCount);

		OnClientLeave(id);

		closesocket(session->socket);

		session->isSending = false;


		while (1)
		{
			if (session->sendQ.Dequeue(&dummy) == false)
			{
				break;
			}

			/*USHORT ret = InterlockedIncrement16((SHORT*)&g_debugPacket2);
			g_sessionDebugs2[ret] = dummy;*/
			dummy->SubRef();
		}
		session->recvQ.ClearBuffer();

		ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));

		DeleteSessionData(id);
	}

	void CLF_LanServer::AcceptProc()
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET client = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (client == INVALID_SOCKET)
		{
			int err = WSAGetLastError();

			if (err == WSAENOTSOCK || err == WSAEINTR)
			{
				CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Listen Socket Close [Error: %d]", err);

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

	CLF_LanServer::Session* CLF_LanServer::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
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

	void CLF_LanServer::GQCS()
	{
		while (1)
		{
			DWORD transferredSize = 0;
			Session* completionKey = nullptr;
			WSAOVERLAPPED* pOverlapped = nullptr;
			Session* session = nullptr;

			BOOL gqcsRet = GetQueuedCompletionStatus(mHcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);
#ifdef PROFILE
			CProfiler::Begin(L"GQCS_Net");
#endif // PROFILE

			// ECHO Server End
			if (transferredSize == 0 && (PULONG_PTR)completionKey == nullptr && pOverlapped == nullptr)
			{
				PostQueuedCompletionStatus(mHcp, 0, 0, 0);

				return;
			}

			if (pOverlapped == nullptr) // I/O Fail
			{
				OnError(10000, L"IOCP Error");

				return;
			}

			session = (Session*)completionKey;

			if (transferredSize != 0)
			{
				if (pOverlapped == &session->recvOverlapped) // Recv
				{
#ifdef PROFILE
					CProfiler::End(L"WSARecv");
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
					CProfiler::End(L"WSASend");
					CProfiler::Begin(L"CompleteSend");
					CompleteSend(session, transferredSize);
					CProfiler::End(L"CompleteSend");
#else
					CompleteSend(session, transferredSize);
#endif // PROFILE
				}
			}

			if (transferredSize == 0 && pOverlapped == (LPOVERLAPPED)1)
			{
#ifdef PROFILE
				CProfiler::Begin(L"SendPost");
				SendPost(session);
				CProfiler::End(L"SendPost");
#else
				SendPost(session);
#endif // PROFILE
			}

			DecrementIOProc(session, 10000);
#ifdef PROFILE
			CProfiler::End(L"GQCS_Net");
#endif // PROFILE
		}
	}

	void CLF_LanServer::CompleteRecv(Session* session, DWORD transferredSize)
	{
		session->recvQ.MoveRear(transferredSize);
		USHORT len;
		DWORD count = 0;
		bool status = true;

		while (count < transferredSize)
		{
			if (session->recvQ.GetUseSize() <= sizeof(USHORT))
				break;

			session->recvQ.Peek((char*)&len, sizeof(USHORT));

			if (len > CLanPacket::eBUFFER_DEFAULT)
			{
				status = false;
				break;
			}

			if (session->recvQ.GetUseSize() < (sizeof(USHORT) + len))
				break;

			//CProfiler::Begin(L"ALLOC");
			CLanPacket* packet = CLanPacket::AllocAddRef();
			//CProfiler::End(L"ALLOC");

			memcpy_s(packet->GetZeroPtr(), sizeof(USHORT), (char*)&len, sizeof(USHORT));

			InterlockedIncrement(&recvTPS);

			session->recvQ.MoveFront(sizeof(USHORT));

			int ret = session->recvQ.Dequeue(packet->GetFrontPtr(), (int)len);

			packet->MoveRear(ret);
			OnRecv(session->sessionID, packet); // -> SendPacket

			count += (ret + sizeof(USHORT));
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

	void CLF_LanServer::CompleteSend(Session* session, DWORD transferredSize)
	{
		CLanPacket* packet;
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

	void CLF_LanServer::CloseSessions()
	{
	}

	void CLF_LanServer::InitializeEmptyIndex()
	{
		for (u_short i = mMaxClient; i > 0; --i)
		{
			mEmptyIndexes.Push(i - 1);
		}
	}

	SESSION_ID CLF_LanServer::GenerateSessionID()
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

	u_short CLF_LanServer::GetIndexFromSessionNo(SESSION_ID sessionNo)
	{
		return (u_short)(sessionNo >> (8 * 6));
	}

	u_int64 CLF_LanServer::GetLowNumFromSessionNo(SESSION_ID sessionNo)
	{
		return sessionNo & 0xffffffffffff;
	}

	ULONG CLF_LanServer::GetSessionIP(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray[index].ip;
	}

	void CLF_LanServer::SetZeroCopy(bool on)
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

	void CLF_LanServer::SetNagle(bool on)
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

	void CLF_LanServer::MonitorProc()
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

	void CLF_LanServer::QuitServer()
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

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Quit CLanServer");
	}

	CLF_LanServer::CLF_LanServer()
	{
	}

	CLF_LanServer::~CLF_LanServer()
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
		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CLanServer End");
	}

	bool CLF_LanServer::Start()
	{
		if (mbBegin == true)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running");
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CLanServer Begin");

		if (!CreateListenSocket())
		{
			return false;
		}

		SetEvent(mBeginEvent);

		return true;
	}

	void CLF_LanServer::Stop()
	{
		mbBegin = false;
		BOOL ret;

		closesocket(mListenSocket);

		for (USHORT i = 0; i < mMaxClient; ++i)
		{
			ret = CancelIoEx((HANDLE)mSessionArray[i].socket, nullptr);
			//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Cancel Ret: %d, Err: %d", ret, GetLastError());
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Stop CLanServer");
	}

	void CLF_LanServer::Begin()
	{
		Init();
		BeginThreads();
	}

	void CLF_LanServer::SetServerIP(const WCHAR* server)
	{
		wcscpy_s(mBindIP, _countof(mBindIP), server);
	}

	void CLF_LanServer::SetServerPort(USHORT port)
	{
		mPort = port;
	}

	bool CLF_LanServer::Disconnect(SESSION_ID SessionID)
	{
		Session* session = FindSession(SessionID);
		BOOL ret;

		IncrementIOProc(session, 40000);

		if (session->ioBlock.releaseCount.isReleased == 1 || SessionID != session->sessionID)
		{
			DecrementIOProc(session, 40020);

			return false;
		}

		ret = CancelIoEx((HANDLE)session->socket, nullptr);

		DecrementIOProc(session, 40040);

		return ret;
	}

	void CLF_LanServer::SendPacket(SESSION_ID SessionID, CLanPacket* packet)
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
		/*ioDebugLog(20010, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
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

	void CLF_LanServer::SendPacketToWorker(SESSION_ID SessionID, CLanPacket* packet)
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
		/*ioDebugLog(20010, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		packet->AddRef();
		session->sendQ.Enqueue(packet);

		IncrementIOProc(session, 20010);
		PostQueuedCompletionStatus(mHcp, 0, (ULONG_PTR)session, (LPOVERLAPPED)1);

		DecrementIOProc(session, 20020);
	}
}
