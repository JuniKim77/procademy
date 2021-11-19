#include "CNetServerNoLock.h"
#include "CLogger.h"
#include "CNetPacket.h"
#include "CCrashDump.h"
#include "CProfiler.h"
#include "TextParser.h"

struct packetDebug
{
	int			logicId;
	DWORD		threadId;
	void*		pChunk;
	int			allocCount;
	void*		pPacket;
	LONG		freeCount;
};

struct ioDebug
{
	int			logicId;
	UINT64		sessionID;
	SHORT		released;
	SHORT		ioCount;
	DWORD		threadId;
};

USHORT g_debugIdx = 0;
packetDebug g_packetDebugs[USHRT_MAX + 1] = { 0, };
USHORT g_debugPacket = 0;
procademy::CNetPacket* g_sessionDebugs[USHRT_MAX + 1] = { 0, };
USHORT g_ioIdx = 0;
ioDebug g_ioDebugs[USHRT_MAX + 1] = { 0, };

void ioDebugLog(
	int			logicId,
	DWORD		threadId,
	UINT64		sessionID,
	SHORT		ioCount,
	SHORT		released
)
{
	USHORT index = (USHORT)InterlockedIncrement16((short*)&g_ioIdx);

	g_ioDebugs[index].logicId = logicId;
	g_ioDebugs[index].sessionID = sessionID;
	g_ioDebugs[index].ioCount = ioCount;
	g_ioDebugs[index].released = released;
	g_ioDebugs[index].threadId = threadId;
}

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

	Session* CNetServerNoLock::FindSession(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		return mSessionArray + index;
	}

	void CNetServerNoLock::InsertSessionData(Session* session)
	{
		u_short index = GetIndexFromSessionNo(session->sessionID);

		mSessionArray[index] = *session;
	}

	void CNetServerNoLock::DeleteSessionData(SESSION_ID sessionNo)
	{
		u_short index = GetIndexFromSessionNo(sessionNo);

		mEmptyIndexes.Push(index);
	}

	void CNetServerNoLock::UpdateSessionData(SESSION_ID sessionNo, Session* session)
	{
	}

	bool CNetServerNoLock::CreateIOCP()
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
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		return true;
	}

	bool CNetServerNoLock::CreateListenSocket()
	{
		WSADATA			wsa;
		SOCKADDR_IN		addr;
		LINGER			optval;

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

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Create\n");

		addr.sin_family = AF_INET;
		addr.sin_port = htons(mPort);
		InetPton(AF_INET, mBindIP, &addr.sin_addr);

		int bindRet = bind(mListenSocket, (SOCKADDR*)&addr, sizeof(addr));

		if (bindRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Bind [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Bind\n");

		// 백로그 길이
		int listenRet = listen(mListenSocket, mMaxClient);

		if (listenRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Listen [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Set Listening\n");

		// TimeWait Zero
		optval.l_onoff = 1;
		optval.l_linger = 0;

		int timeOutnRet = setsockopt(mListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));
		if (timeOutnRet == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Listen Socket Linger [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Option : TimeOut Zero\n");

		SetZeroCopy(mbZeroCopy);
		SetNagle(mbNagle);

		return true;
	}

	bool CNetServerNoLock::BeginThreads()
	{
		BYTE i = 0;

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
		mNumThreads++;

		for (; i <= mWorkerThreadNum; ++i)
		{
			mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
		}

		mNumThreads += mWorkerThreadNum;

		mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, MonitoringThread, this, 0, nullptr);
		mNumThreads++;

		return true;
	}

	unsigned int __stdcall CNetServerNoLock::WorkerThread(LPVOID arg)
	{
		CNetServerNoLock* server = (CNetServerNoLock*)arg;

		while (!server->mbExit)
		{
			server->CompleteMessage();
		}

		return 0;
	}

	unsigned int __stdcall CNetServerNoLock::AcceptThread(LPVOID arg)
	{
		CNetServerNoLock* server = (CNetServerNoLock*)arg;

		while (!server->mbExit)
		{
			if (server->mbBegin)
			{
				server->AcceptProc();
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

				DecrementIOProc(session, 20050);

				ret = false;
			}

			break;
		} while (1);

		return ret;
	}

	void CNetServerNoLock::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
	{
		CProfiler::Begin(L"SetWSABuf");
		
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

		CProfiler::End(L"SetWSABuf");
	}

	void CNetServerNoLock::IncrementIOProc(Session* session, int logic)
	{
		InterlockedIncrement(&session->ioBlock.ioCount);
		/*ioDebugLog(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
	}

	void CNetServerNoLock::DecrementIOProc(Session* session, int logic)
	{
		SessionIoCount ret;

		ret.ioCount = InterlockedDecrement(&session->ioBlock.ioCount);
		//if (ret.releaseCount.count <= 0)
		//{
		//	/*ioDebugLog(logic, GetCurrentThreadId(), session->sessionID & 0xffffffff,
		//		session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		//	int test = 0;
		//}

		if (ret.releaseCount.count < 0)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"[SessionID: %llu] [ioCount: %d] [isSending: %d] [isAlive: %d] [recv: %d] [Send: %d]\n\n",
				session->sessionID & 0xffffffffff,
				session->ioBlock.ioCount,
				session->isSending,
				session->bIsAlive,
				session->recvQ.GetUseSize(),
				session->sendQ.GetSize());

			CRASH();
		}

		if (ret.ioCount == 0)
		{
			ReleaseProc(session);
		}
	}

	void CNetServerNoLock::ReleaseProc(Session* session)
	{
		//CProfiler::Begin(L"RELEASEPROC");
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
		InterlockedIncrement((LONG*)&disconnectCount);
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
			
			/*USHORT ret = InterlockedIncrement16((SHORT*)&g_debugPacket2);
			g_sessionDebugs2[ret] = dummy;*/
			dummy->SubRef();
		}
		session->recvQ.ClearBuffer();

		ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));

		DeleteSessionData(id);
		//CProfiler::End(L"RELEASEPROC");
	}

	void CNetServerNoLock::AcceptProc()
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);

		SOCKET client = accept(mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);

		if (client == INVALID_SOCKET)
		{
			int err = WSAGetLastError();

			if (err == WSAENOTSOCK)
			{
				CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Listen Socket Close [Error: %d]\n", err);

				return;
			}

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Listen Socket [Error: %d]\n", err);

			return;
		}

		if (OnConnectionRequest(clientAddr.sin_addr.S_un.S_addr, clientAddr.sin_port) == false)
		{
			WCHAR IP[16] = { 0, };

			InetNtop(AF_INET, &clientAddr.sin_addr.S_un.S_addr, IP, 16);

			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept Denied [IP: %s] [Port: %u]\n",
				IP, ntohs(clientAddr.sin_port));

			closesocket(client);

			return;
		}

		Session* session = CreateSession(client, clientAddr);

		if (session == nullptr)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create Session Fail\n");
			return;
		}

		//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Socket Accept [IP: %s] [Port: %u]\n",
		//    IP, ntohs(clientAddr.sin_port));

		IncrementIOProc(session, 30000);
		session->ioBlock.releaseCount.isReleased = 0;

		/*ioDebug(10020, GetCurrentThreadId(), session->sessionID & 0xffffffff,
			session->ioBlock.releaseCount.count, session->ioBlock.releaseCount.isReleased);*/
		session->bIsAlive = true;
		OnClientJoin(session->sessionID);
		CProfiler::Begin(L"RecvPost");
		RecvPost(session, true);
		CProfiler::End(L"RecvPost");
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
		session->bIsAlive = true;

		HANDLE hResult = CreateIoCompletionPort((HANDLE)client, mHcp, (ULONG_PTR)session, 0);

		if (hResult == NULL)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);

			return nullptr;
		}

		//InterlockedIncrement((LONG*)&mMonitor.acceptCount);
		acceptTotal++;
		acceptTPS++;

		return session;
	}

	void CNetServerNoLock::CompleteMessage()
	{
		while (1)
		{
			DWORD transferredSize = 0;
			Session* completionKey = nullptr;
			WSAOVERLAPPED* pOverlapped = nullptr;
			Session* session = nullptr;
			
			BOOL gqcsRet = GetQueuedCompletionStatus(mHcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);
			CProfiler::Begin(L"GQCS_Net");

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
					CProfiler::Begin(L"CompleteRecv");
					CompleteRecv(session, transferredSize);
					CProfiler::End(L"CompleteRecv");
				}
				else // Send
				{
					CProfiler::Begin(L"CompleteSend");
					CompleteSend(session, transferredSize);
					CProfiler::End(L"CompleteSend");
				}
			}

			if (transferredSize == 0 && pOverlapped == (LPOVERLAPPED)1)
			{
				CProfiler::Begin(L"SendPost");
				SendPost(session);
				CProfiler::End(L"SendPost");
			}

			DecrementIOProc(session, 10000);
			CProfiler::End(L"GQCS_Net");
		}
	}

	void CNetServerNoLock::CompleteRecv(Session* session, DWORD transferredSize)
	{
		session->recvQ.MoveRear(transferredSize);
		CNetPacket::st_Header header;
		DWORD count = 0;

		while (count < transferredSize)
		{
			if (session->bIsAlive == false)
			{
				return;
			}

			if (session->recvQ.GetUseSize() <= CNetPacket::HEADER_MAX_SIZE)
				break;

			session->recvQ.Peek((char*)&header, CNetPacket::HEADER_MAX_SIZE);

			if (session->recvQ.GetUseSize() < (CNetPacket::HEADER_MAX_SIZE + header.len))
				break;

			//CProfiler::Begin(L"ALLOC");
			CNetPacket* packet = CNetPacket::AllocAddRef();
			//CProfiler::End(L"ALLOC");

			memcpy_s(packet->GetZeroPtr(), CNetPacket::HEADER_MAX_SIZE, (char*)&header, CNetPacket::HEADER_MAX_SIZE);

			InterlockedIncrement(&recvTPS);

			session->recvQ.MoveFront(CNetPacket::HEADER_MAX_SIZE);

			int ret = session->recvQ.Dequeue(packet->GetFrontPtr(), (int)header.len);

			packet->MoveRear(ret);
			packet->Decode();
			OnRecv(session->sessionID, packet); // -> SendPacket

			count += (ret + sizeof(SHORT));
			packet->SubRef();
		}

		CProfiler::Begin(L"RecvPost");
		RecvPost(session);
		CProfiler::End(L"RecvPost");
	}

	void CNetServerNoLock::CompleteSend(Session* session, DWORD transferredSize)
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

	SESSION_ID CNetServerNoLock::GenerateSessionID()
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

	u_short CNetServerNoLock::GetIndexFromSessionNo(SESSION_ID sessionNo)
	{
		return (u_short)(sessionNo >> (8 * 6));
	}

	u_int64 CNetServerNoLock::GetLowNumFromSessionNo(SESSION_ID sessionNo)
	{
		return sessionNo & 0xffffffffffff;
	}

	void CNetServerNoLock::SetZeroCopy(bool on)
	{
		int optNum = on ? 0 : SEND_BUF_SIZE;

		if (setsockopt(mListenSocket, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);

			return;
		}

		if (on)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Zero Copy On\n");
		}
		else
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Zero Copy Off\n");
		}
	}

	void CNetServerNoLock::SetNagle(bool on)
	{
		BOOL optval = on;

		if (setsockopt(mListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]\n", WSAGetLastError());
			closesocket(mListenSocket);

			return;
		}

		if (on)
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Nagle On\n");
		}
		else
		{
			CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Listen Socket Nagle Off\n");
		}
	}

	void CNetServerNoLock::NetworkMonitorProc()
	{
		HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

		while (!mbExit)
		{
			DWORD retval = WaitForSingleObject(dummyEvent, 1000);

			if (retval == WAIT_TIMEOUT && mbMonitoring == true)
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

	void CNetServerNoLock::QuitServer()
	{
		Stop();

		PostQueuedCompletionStatus(mHcp, 0, 0, 0);

		mbExit = true;

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Quit CNetServer\n");

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

	CNetServerNoLock::CNetServerNoLock()
	{
		LoadInitFile(L"ChatServer.cnf");

		mhThreads = new HANDLE[(long long)mWorkerThreadNum + 2];
		mSessionArray = (Session*)_aligned_malloc(sizeof(Session) * mMaxClient, 64);
		for (int i = 0; i < mMaxClient; ++i)
		{
			new (&mSessionArray[i]) (Session);
		}

		InitializeEmptyIndex();
		BeginThreads();
		CreateIOCP();
		CLogger::SetDirectory(L"_log");
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

	bool CNetServerNoLock::Start()
	{
		if (mbBegin == true)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running\n");
			return false;
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServer Begin\n");

		if (!CreateListenSocket())
		{
			return false;
		}

		mbBegin = true;

		return true;
	}

	void CNetServerNoLock::Stop()
	{
		mbBegin = false;
		BOOL ret;

		closesocket(mListenSocket);

		for (USHORT i = 0; i < mMaxClient; ++i)
		{
			ret = CancelIoEx((HANDLE)mSessionArray[i].socket, nullptr);
			//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Cancel Ret: %d, Err: %d\n", ret, GetLastError());
		}

		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Stop CNetServer\n");
	}

	bool CNetServerNoLock::Disconnect(SESSION_ID SessionID)
	{
		Session* session = FindSession(SessionID);
		BOOL ret;

		if (session == nullptr)
			return false;

		return CancelIoEx((HANDLE)session->socket, nullptr);
	}

	void CNetServerNoLock::SendPacket(SESSION_ID SessionID, CNetPacket* packet)
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

		CProfiler::Begin(L"SendPost");
		SendPost(session);
		CProfiler::End(L"SendPost");

		DecrementIOProc(session, 20020);
	}

	void CNetServerNoLock::SendPacketToWorker(SESSION_ID SessionID, CNetPacket* packet)
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

	void CNetServerNoLock::LoadInitFile(const WCHAR* fileName)
	{
		TextParser  tp;
		int         num;
		WCHAR       buffer[MAX_PARSER_LENGTH];

		tp.LoadFile(fileName);

		tp.GetValue(L"BIND_IP", mBindIP);

		tp.GetValue(L"BIND_PORT", &num);
		mPort = (u_short)num;

		tp.GetValue(L"IOCP_WORKER_THREAD", &num);
		mWorkerThreadNum = (BYTE)num;

		tp.GetValue(L"IOCP_ACTIVE_THREAD", &num);
		mActiveThreadNum = (BYTE)num;

		tp.GetValue(L"CLIENT_MAX", &num);
		mMaxClient = (u_short)num;

		tp.GetValue(L"NAGLE", buffer);
		if (wcscmp(L"TRUE", buffer) == 0)
			mbNagle = true;
		else
			mbNagle = false;

		tp.GetValue(L"LOG_LEVEL", buffer);
		if (wcscmp(buffer, L"DEBUG") == 0)
			CLogger::setLogLevel(dfLOG_LEVEL_DEBUG);
		else if (wcscmp(buffer, L"WARNING") == 0)
			CLogger::setLogLevel(dfLOG_LEVEL_SYSTEM);
		else if (wcscmp(buffer, L"ERROR") == 0)
			CLogger::setLogLevel(dfLOG_LEVEL_ERROR);
	}
}
