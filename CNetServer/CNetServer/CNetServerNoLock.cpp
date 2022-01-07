#include "CNetServerNoLock.h"
#include "CLogger.h"
#include "CNetPacket.h"
#include "CCrashDump.h"
#include "CProfiler.h"

struct packetDebug
{
	int			logicId;
	DWORD		threadId;
	void*		pChunk;
	void*		pPacket;
	int			allocCount;
	LONG		freeCount;
};

USHORT g_debugIdx = 0;
packetDebug g_packetDebugs[USHRT_MAX + 1] = { 0, };

void packetLog(
	int			logicId = -9999,
	DWORD		threadId = 0,
	void*		pChunk = nullptr,
	void*		pPacket = nullptr,
	int			allocCount = -9999,
	LONG		freeCount = 9999
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debugIdx);

	g_packetDebugs[index].logicId = logicId;
	g_packetDebugs[index].threadId = threadId;
	g_packetDebugs[index].pChunk = pChunk;
	g_packetDebugs[index].pPacket = pPacket;
	g_packetDebugs[index].allocCount = allocCount;
	g_packetDebugs[index].freeCount = freeCount;
}

namespace procademy
{
	struct packetDebug
	{
		DWORD packetNum;
		u_int64 sessionID;
		int logicId;
		DWORD threadId;
	};

	USHORT g_debug_index = 0;
	packetDebug g_debugs[USHRT_MAX + 1] = { 0, };

	void packDebug(
		int logicId,
		DWORD threadId,
		u_int64 sessionID,
		DWORD packetNum
	)
	{
		USHORT index = (USHORT)InterlockedIncrement16((short*)&g_debug_index);

		g_debugs[index].logicId = logicId;
		g_debugs[index].threadId = threadId;
		g_debugs[index].sessionID = sessionID;
		g_debugs[index].packetNum = packetNum;
	}

	Session* CNetServerNoLock::FindSession(u_int64 sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray + index;
	}

	void CNetServerNoLock::InsertSessionData(Session* session)
	{
		u_short index = GetIndexFromSessionNo(session->sessionID);

		mSessionArray[index] = *session;
	}

	void CNetServerNoLock::DeleteSessionData(u_int64 sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		mEmptyIndexes.Push(index);
	}

	void CNetServerNoLock::UpdateSessionData(u_int64 sessionNo, Session* session)
	{
	}

	bool CNetServerNoLock::CreateListenSocket()
	{
		WSADATA wsa;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup [Error: %d]\n", WSAGetLastError());
			return false;
		}

		mListenSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (mListenSocket == INVALID_SOCKET)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create socket [Error: %d]\n", WSAGetLastError());
			return false;
		}

		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(mPort);
		addr.sin_addr.S_un.S_addr = htonl(mBindIP);

		int bindRet = bind(mListenSocket, (SOCKADDR*)&addr, sizeof(addr));

		if (bindRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Bind [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		// 백로그 길이?
		int listenRet = listen(mListenSocket, SOMAXCONN);

		if (listenRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Listen [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		// 논리 코어 개수 확인 로직
		SYSTEM_INFO si;
		GetSystemInfo(&si);

		if (mMaxRunThreadSize > si.dwNumberOfProcessors)
		{
			CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Setting: Max Running thread is larger than the number of processors");
		}

		mHcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, mMaxRunThreadSize);

		if (mHcp == NULL)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		return true;
	}

	bool CNetServerNoLock::BeginThreads()
	{
		BYTE i = 0;

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
		mNumThreads++;

		for (; i <= mWorkerThreadSize; ++i)
		{
			mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
		}

		mNumThreads += mWorkerThreadSize;

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, MonitoringThread, this, 0, nullptr);
		mNumThreads++;

		return true;
	}

	unsigned int __stdcall CNetServerNoLock::WorkerThread(LPVOID arg)
	{
		CNetServerNoLock* server = (CNetServerNoLock*)arg;

		while (1)
		{
			if (server->OnCompleteMessage() == false)
			{
				break;
			}
		}

		return 0;
	}

	unsigned int __stdcall CNetServerNoLock::AcceptThread(LPVOID arg)
	{
		CNetServerNoLock* server = (CNetServerNoLock*)arg;

		while (1)
		{
			if (server->AcceptProc() == false)
			{
				break;
			}
		}

		return 0;
	}

	unsigned int __stdcall CNetServerNoLock::MonitoringThread(LPVOID arg)
	{
		CNetServerNoLock* server = (CNetServerNoLock*)arg;

		server->NetworkMonitorProc();

		return 0;
	}

	bool CNetServerNoLock::RecvPost(Session* session, bool isAccepted)
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

			DecrementIOProc(session, 10050);

			return false;
		}

		return true;
	}

	bool CNetServerNoLock::SendPost(Session* session)
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
				if (InterlockedExchange8((char*)&session->isSending, false) == false)
				{
					CRASH();
				}

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

				DecrementIOProc(session, 20050);

				ret = false;
			}

			break;
		} while (1);

		return ret;
	}

	void CNetServerNoLock::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
	{
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
			CNetPacket* packetBufs[100];
			DWORD snapSize = session->sendQ.GetSize();

			if (snapSize > 100)
				snapSize = 100;

			if (session->sendQ.Peek(packetBufs, snapSize) != snapSize)
				CRASH();

			for (DWORD i = 0; i < snapSize; ++i)
			{
				bufs[i].buf = packetBufs[i]->GetZeroPtr();
				bufs[i].len = packetBufs[i]->GetSize();
			}

			session->numSendingPacket = snapSize;
		}
	}

	void CNetServerNoLock::IncrementIOProc(Session* session, int logic)
	{
		InterlockedIncrement(&session->ioBlock.ioCount);
		/*ioDebug(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
	}

	void CNetServerNoLock::DecrementIOProc(Session* session, int logic)
	{
		SessionIoCount ret;

		ret.ioCount = InterlockedDecrement(&session->ioBlock.ioCount);
		if (ret.releaseCount.count <= 0)
		{
			/*packDebug(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
				session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		}

		if (ret.releaseCount.count < 0)
		{
			WCHAR temp[1024] = { 0, };
			swprintf_s(temp, 1024, L"[SessionID: %llu] [ioCount: %d] [isSending: %d] [isAlive: %d] [recv: %d] [Send: %d]\n\n",
				session->sessionID & 0xffffffffff,
				session->ioBlock.ioCount,
				session->isSending,
				session->bIsAlive,
				session->recvQ.GetUseSize(),
				session->sendQ.GetSize());

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"%s", temp);

			CRASH();
		}

		if (ret.ioCount == 0)
		{
			ReleaseProc(session);
		}
	}

	void CNetServerNoLock::ReleaseProc(Session* session)
	{
		CProfiler::Begin(L"RELEASEPROC");
		SessionIoCount released;
		CNetPacket* dummy;

		released.ioCount = 0;
		released.releaseCount.isReleased = 1;

		if (InterlockedCompareExchange(&session->ioBlock.ioCount, released.ioCount, 0) != 0)
		{
			return;
		}

		u_int64 id = session->sessionID;
		session->sessionID = 0;
		InterlockedIncrement((LONG*)&mMonitor.disconnectCount);
		session->bIsAlive = false;

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

		//LockSessionMap();
		DeleteSessionData(id);
		//UnlockSessionMap();
		CProfiler::End(L"RELEASEPROC");
	}

	bool CNetServerNoLock::AcceptProc()
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET client = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (client == INVALID_SOCKET)
		{
			int err = WSAGetLastError();

			if (err == WSAENOTSOCK)
			{
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"ListenSocket [Error: %d]\n", err);

				return false;
			}

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept [Error: %d]\n", err);

			return false;
		}

		if (OnConnectionRequest(clientAddr.sin_addr.S_un.S_addr, clientAddr.sin_port) == false)
		{
			WCHAR IP[16] = { 0, };

			InetNtop(AF_INET, &clientAddr.sin_addr.S_un.S_addr, IP, 16);

			CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Socket Accept Denied [IP: %s] [Port: %u]\n",
				IP, ntohs(clientAddr.sin_port));

			closesocket(client);

			return false;
		}

		if (mbZeroCopy)
		{
			int optNum = 0;
			if (setsockopt(client, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
			{
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy [Error: %d]\n", WSAGetLastError());
				closesocket(client);

				return false;
			}
		}

		if (mbNagle)
		{
			BOOL optval = mbNagle;

			if (setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
			{
				CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]\n", WSAGetLastError());
				closesocket(client);

				return false;
			}
		}

		Session* session = CreateSession(client, clientAddr);

		if (session == nullptr)
		{
			return false;
		}

		//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Socket Accept [IP: %s] [Port: %u]\n",
		//    IP, ntohs(clientAddr.sin_port));

		IncrementIOProc(session, 30000);

		session->ioBlock.releaseCount.isReleased = 0;

		/*ioDebug(10020, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		session->bIsAlive = true;
		OnClientJoin(session->sessionID);
		RecvPost(session, true);

		return true;
	}

	Session* CNetServerNoLock::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
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
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);

			return nullptr;
		}

		InterlockedIncrement((LONG*)&mMonitor.acceptCount);

		return session;
	}

	bool CNetServerNoLock::OnCompleteMessage()
	{
		DWORD transferredSize = 0;
		Session* completionKey = nullptr;
		WSAOVERLAPPED* pOverlapped = nullptr;
		Session* session = nullptr;

		BOOL gqcsRet = GetQueuedCompletionStatus(mHcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);

		// ECHO Server End
		if (transferredSize == 0 && (PULONG_PTR)completionKey == nullptr && pOverlapped == nullptr)
		{
			PostQueuedCompletionStatus(mHcp, 0, 0, 0);

			return false;
		}

		if (pOverlapped == nullptr) // I/O Fail
		{
			OnError(10000, L"IOCP Error");

			return false;
		}

		session = (Session*)completionKey;

		if (transferredSize != 0) // Normal close
		{
			if (pOverlapped == &session->recvOverlapped) // Recv
			{
				//CProfiler::Begin(L"CompleteRecv");
				CompleteRecv(session, transferredSize);
				//CProfiler::End(L"CompleteRecv");
			}
			else // Send
			{
				//CProfiler::Begin(L"CompleteSend");
				CompleteSend(session, transferredSize);
				//CProfiler::End(L"CompleteSend");
			}
		}

		DecrementIOProc(session, 10000);

		return true;
	}

	void CNetServerNoLock::CompleteRecv(Session* session, DWORD transferredSize)
	{
		session->recvQ.MoveRear(transferredSize);
		DWORD count = 0;

		while (count < transferredSize)
		{
			if (session->bIsAlive == false)
			{
				return;
			}
			CProfiler::Begin(L"ALLOC");
			CNetPacket* packet = CNetPacket::AllocAddRef();
			CProfiler::End(L"ALLOC");
			InterlockedIncrement(&mMonitor.recvTPS);
			InterlockedIncrement(&mMonitor.sendTPS);

			if (session->recvQ.GetUseSize() <= sizeof(SHORT))
				break;
			session->recvQ.Peek((char*)packet->GetZeroPtr(), sizeof(SHORT));

			if (session->recvQ.GetUseSize() < (int)(sizeof(SHORT) + packet->GetPacketSize()))
				break;

			session->recvQ.MoveFront(sizeof(SHORT));

			int ret = session->recvQ.Dequeue(packet->GetFrontPtr(), (int)packet->GetPacketSize());

			packet->MoveRear(ret);
			OnRecv(session->sessionID, packet); // -> SendPacket

			count += (ret + sizeof(SHORT));
			packet->SubRef();
		}

		RecvPost(session);
	}

	void CNetServerNoLock::CompleteSend(Session* session, DWORD transferredSize)
	{
		CNetPacket* packet;

		for (int i = 0; i < session->numSendingPacket; ++i)
		{
			session->sendQ.Dequeue(&packet);

			packet->SubRef();
			packet = nullptr;
		}

		session->numSendingPacket = 0;
		session->isSending = false;
		CProfiler::Begin(L"SendPost");
		SendPost(session);
		CProfiler::End(L"SendPost");
	}

	void CNetServerNoLock::CloseSessions()
	{
	}

	void CNetServerNoLock::InitializeEmptyIndex()
	{
		for (u_short i = mMaxClient; i > 0; --i)
		{
			mEmptyIndexes.Push(i - 1);
		}

		//InitializeSRWLock(&mStackLock);
	}

	u_int64 CNetServerNoLock::GenerateSessionID()
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

	u_short CNetServerNoLock::GetIndexFromSessionNo(u_int64 sessionNo)
	{
		return (u_short)(sessionNo >> (8 * 6));
	}

	void CNetServerNoLock::NetworkMonitorProc()
	{
		HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

		while (!mbIsQuit)
		{
			DWORD retval = WaitForSingleObject(dummyEvent, 1000);

			if (retval == WAIT_TIMEOUT)
			{
				mMonitor.prevRecvTPS = mMonitor.recvTPS;
				mMonitor.prevSendTPS = mMonitor.sendTPS;

				mMonitor.recvTPS = 0;
				mMonitor.sendTPS = 0;
			}
		}

		CloseHandle(dummyEvent);
	}

	void CNetServerNoLock::QuitServer()
	{
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Exit\n");

		PostQueuedCompletionStatus(mHcp, 0, 0, 0);

		mbIsQuit = true;

		closesocket(mListenSocket);
	}

	CNetServerNoLock::~CNetServerNoLock()
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
		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Lib End\n");
	}

	bool CNetServerNoLock::Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
	{
		CLogger::SetDirectory(L"_log");

		if (mbIsRunning == true)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running\n");
			return false;
		}

		mPort = port;
		mBindIP = ip;
		mWorkerThreadSize = createThread;
		mMaxRunThreadSize = runThread;
		mMaxClient = maxClient;
		mbNagle = nagle;

		mhThreads = new HANDLE[(long long)createThread + 2];
		mSessionArray = (Session*)_aligned_malloc(sizeof(Session) * maxClient, 64);
		for (int i = 0; i < maxClient; ++i)
		{
			new (&mSessionArray[i]) (Session);
		}
		//mSessionArray = new Session[maxClient];

		if (!CreateListenSocket())
		{
			return false;
		}

		BeginThreads();

		mbIsRunning = true;

		InitializeEmptyIndex();

		return true;
	}

	bool CNetServerNoLock::Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
	{
		return Start(port, INADDR_ANY, createThread, runThread, nagle, maxClient);
	}

	void CNetServerNoLock::Stop()
	{
	}

	int CNetServerNoLock::GetSessionCount()
	{
		return 0;
	}

	void CNetServerNoLock::WaitForThreadsFin()
	{
		DWORD waitResult = WaitForMultipleObjects(mNumThreads, mhThreads, TRUE, INFINITE);

		switch (waitResult)
		{
		case WAIT_FAILED:
			wprintf_s(L"Main Thread Handle Error\n");
			break;
		case WAIT_TIMEOUT:
			wprintf_s(L"Main Thread Timeout Error\n");
			break;
		case WAIT_OBJECT_0:
			wprintf_s(L"None Error\n");
			break;
		default:
			break;
		}
	}

	bool CNetServerNoLock::Disconnect(SESSION_ID SessionID)
	{
		return false;
	}

	void CNetServerNoLock::SendPacket(SESSION_ID SessionID, CNetPacket* packet)
	{
		Session* session = FindSession(SessionID);

		IncrementIOProc(session, 20000);

		if (SessionID != session->sessionID)
		{
			DecrementIOProc(session, 20020);

			return;
		}

		/*st_NETWORK_HEADER header;

		header.byCode = dfNETWORK_CODE;
		header.wPayloadSize = packet->GetSize();*/
		packet->AddRef();
		char* buf = packet->GetBufferPtr();
		packDebug(10000, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			*((DWORD*)(buf + 2)));
		session->sendQ.Enqueue(packet);

		CProfiler::Begin(L"SendPost");
		bool ret = SendPost(session);
		CProfiler::End(L"SendPost");

		if (ret)
		{
			DecrementIOProc(session, 20010);
		}
	}
}
