//#define SPIN_LOCK

#include "CLanServerNoLock.h"
#include "CLogger.h"
#include "CPacket.h"
#include "CCrashDump.h"

CLanServerNoLock::Session* CLanServerNoLock::FindSession(u_int64 sessionNo)
{
	u_short index = GetIndexFromSessionNo(sessionNo);

	return mSessionArray[index];
}

void CLanServerNoLock::InsertSessionData(Session* session)
{
	u_short index = GetIndexFromSessionNo(session->sessionID);

	mSessionArray[index] = session;
}

void CLanServerNoLock::DeleteSessionData(u_int64 sessionNo)
{
	u_short index = GetIndexFromSessionNo(sessionNo);

	mSessionPool->Lock(false);
	mSessionPool->Free(mSessionArray[index]);
	mSessionPool->Unlock(false);

	mSessionArray[index] = nullptr;
	LockStack();
	mEmptyIndexes.push(index);
	UnlockStack();
}

void CLanServerNoLock::UpdateSessionData(u_int64 sessionNo, Session* session)
{
}

bool CLanServerNoLock::CreateListenSocket()
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

bool CLanServerNoLock::BeginThreads()
{
	mhThreads[0] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);
	mNumThreads++;

	for (BYTE i = 1; i <= mWorkerThreadSize; ++i)
	{
		mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
	}

	mNumThreads += mWorkerThreadSize;

	return true;
}

unsigned int __stdcall CLanServerNoLock::WorkerThread(LPVOID arg)
{
	CLanServerNoLock* server = (CLanServerNoLock*)arg;

	while (1)
	{
		if (server->OnCompleteMessage() == false)
		{
			break;
		}
	}

	return 0;
}

unsigned int __stdcall CLanServerNoLock::AcceptThread(LPVOID arg)
{
	CLanServerNoLock* server = (CLanServerNoLock*)arg;

	while (1)
	{
		if (server->AcceptProc() == false)
		{
			break;
		}
	}

	return 0;
}

bool CLanServerNoLock::RecvPost(Session* session)
{
	WSABUF buffers[2];
	DWORD flags = 0;

	SetWSABuf(buffers, session, true);

	int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recv.overlapped, nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"WSARecv ERROR: ", err);

		//LockSession(session);
		bool ret = DecrementProc(session);

		if (ret == false)
		{
			DisconnectProc(session);
		}
		//UnlockSession(session);

		return false;
	}

	return true;
}

int CLanServerNoLock::SendPost(Session* session)
{
	WSABUF buffers[2];

	if (InterlockedExchange8((char*)&session->isSending, true) == true)
	{
		return SEND_POST_SEND_FLAG_ON;
	}

	session->send.queue.Lock(false);
	if (session->send.queue.GetUseSize() == 0)
	{
		InterlockedExchange8((char*)&session->isSending, false);
		session->send.queue.Unlock(false);

		return SEND_POST_RING_QUEUE_EMPTY;
	}

	SetWSABuf(buffers, session, false);
	session->send.queue.Unlock(false);

	if ((InterlockedIncrement16((short*)&session->ioCount) & 0xff) > 2)
	{
		CRASH();
	}
	ZeroMemory(&session->send.overlapped, sizeof(session->send.overlapped));

	int sendRet = WSASend(session->socket, buffers, 2, nullptr, 0, &session->send.overlapped, nullptr);

	if (sendRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return SEND_POST_PACKET_SENT;
		}

		if (DecrementProc(session))
		{
			return SEND_POST_SEND_ERROR;
		}
		else
		{
			return SEND_POST_IO_COUNT_ZERO;
		}
	}

	return SEND_POST_PACKET_SENT;
}

void CLanServerNoLock::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
{
	if (isRecv)
	{
		int dSize = session->recv.queue.DirectEnqueueSize();

		bufs[0].buf = session->recv.queue.GetRearBufferPtr();
		bufs[0].len = dSize;
		bufs[1].buf = session->recv.queue.GetBuffer();
		bufs[1].len = session->recv.queue.GetFreeSize() - dSize;
	}
	else
	{
		int dSize = session->send.queue.DirectDequeueSize();

		bufs[0].buf = session->send.queue.GetFrontBufferPtr();
		bufs[0].len = dSize;
		bufs[1].buf = session->send.queue.GetBuffer();
		bufs[1].len = session->send.queue.GetUseSize() - dSize;
	}
}

bool CLanServerNoLock::DecrementProc(Session* session)
{
	//session->ioCount--;
	int ret = InterlockedDecrement16(&session->ioCount);

	if (ret < 0)
	{
		WCHAR temp[1024] = { 0, };
		swprintf_s(temp, 1024, L"[SessionID: %llu] [ioCount: %d] [isSending: %d] [isAlive: %d] [recv: %d] [Send: %d]\n\n",
			session->sessionID & 0xffffffffff,
			session->ioCount,
			session->isSending,
			session->bIsAlive,
			session->recv.queue.GetUseSize(),
			session->send.queue.GetUseSize());

		CLogger::_Log(dfLOG_LEVEL_ERROR, L"%s", temp);

		CRASH();
	}

	return ret != 0;
}

void CLanServerNoLock::DisconnectProc(Session* session)
{
	session->isDisconnecting = false;
	/*if (InterlockedExchange8((char*)&session->isDisconnecting, true) == true)
	{
		return;
	}*/
	if (session->bIsAlive == false)
	{
		CRASH();
	}

	u_int64 id = session->sessionID;

	OnClientLeave(id);

	closesocket(session->socket);

	//LockSessionMap();
	DeleteSessionData(id);
	//UnlockSessionMap();

	//MonitorLock();
	mMonitor.disconnectCount++;
	//MonitorUnlock();
	session->bIsAlive = false;
	session->isDisconnecting = false;
}

void CLanServerNoLock::PacketProc(Session* session, DWORD msgSize)
{
	session->recv.queue.MoveRear(msgSize);

	int count = 0;
	CPacket packet;

	while (count < msgSize)
	{
		//MonitorLock();
		mMonitor.recvTPS++;
		mMonitor.sendTPS++;
		//MonitorUnlock();

		int ret = session->recv.queue.Dequeue(packet.GetFrontPtr(), 10);

		if (ret != 10)
		{
			wprintf_s(L"Error\n");
		}

		/*int num = *(packet.GetFrontPtr() + 2);

		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"%d\n", num);*/

		packet.MoveRear(ret);

		OnRecv(session->sessionID, &packet);
		//count += (sizeof(header) + header.wPayloadSize);
		count += ret;

		packet.Clear();
	}
}

bool CLanServerNoLock::AcceptProc()
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

		return true;
	}

	if (OnConnectionRequest(clientAddr.sin_addr.S_un.S_addr, clientAddr.sin_port) == false)
	{
		WCHAR IP[16] = { 0, };

		InetNtop(AF_INET, &clientAddr.sin_addr.S_un.S_addr, IP, 16);

		CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Socket Accept Denied [IP: %s] [Port: %u]\n",
			IP, ntohs(clientAddr.sin_port));

		closesocket(client);

		return true;
	}

	if (mbZeroCopy)
	{
		int optNum = 0;
		if (setsockopt(client, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy [Error: %d]\n", WSAGetLastError());
			closesocket(client);

			return true;
		}
	}

	Session* session = CreateSession(client, clientAddr);

	if (session == nullptr)
	{
		return false;
	}

	//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Socket Accept [IP: %s] [Port: %u]\n",
	//    IP, ntohs(clientAddr.sin_port));

	session->ioCount = 1;
	OnClientJoin(session->sessionID);
	RecvPost(session);

	return true;
}

CLanServerNoLock::Session* CLanServerNoLock::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
{
	u_int64 id = GenerateSessionID();

	if (id == 0)
	{
		return nullptr;
	}

	mSessionPool->Lock(false);
	Session* session = mSessionPool->Alloc();
	mSessionPool->Unlock(false);

	session->socket = client;
	session->ip = clientAddr.sin_addr.S_un.S_addr;
	session->port = clientAddr.sin_port;
	session->isSending = false;
	session->sessionID = id;
	session->send.queue.ClearBuffer();
	session->send.queue.InitializeLock();
	session->recv.queue.ClearBuffer();
	session->recv.queue.InitializeLock();
	session->bIsAlive = true;
	session->isDisconnecting = false;

	ZeroMemory(&session->send.overlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&session->recv.overlapped, sizeof(WSAOVERLAPPED));
	InitializeSRWLock(&session->lock);

	HANDLE hResult = CreateIoCompletionPort((HANDLE)client, mHcp, (ULONG_PTR)session, 0);

	if (hResult == NULL)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
		closesocket(mListenSocket);

		return nullptr;
	}

	//LockSessionMap();
	InsertSessionData(session);
	//UnlockSessionMap();

	//MonitorLock();
	mMonitor.acceptCount++;
	//MonitorUnlock();

	return session;
}

bool CLanServerNoLock::OnCompleteMessage()
{
	DWORD transferredSize = 0;
	Session* completionKey = nullptr;
	WSAOVERLAPPED* pOverlapped = nullptr;
	Session* session = nullptr;

	BOOL gqcsRet = GetQueuedCompletionStatus(mHcp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);

	if (transferredSize == 0 && (PULONG_PTR)completionKey == nullptr && pOverlapped == nullptr)
	{
		PostQueuedCompletionStatus(mHcp, 0, 0, 0);

		return false;
	}

	if (pOverlapped == nullptr) // I/O Fail
	{
		return true;
	}

	session = (Session*)completionKey;

	if (transferredSize == 0) // normal close
	{
		//LockSession(session);
		bool ret = DecrementProc(session);
		//UnlockSession(session);

		if (ret == false)
		{
			DisconnectProc(session);
		}

		return true;
	}

	if (pOverlapped == &session->recv.overlapped) // Recv
	{
		PacketProc(session, transferredSize);

		RecvPost(session);
	}
	else // Send
	{
		bool ret = DecrementProc(session);
		int sendRet = 0;

		if (ret)
		{
			session->send.queue.Lock(false);
			session->send.queue.MoveFront(transferredSize);
			session->send.queue.Unlock(false);

			InterlockedExchange8((char*)&session->isSending, false);
			sendRet = SendPost(session);

			if (sendRet == SEND_POST_IO_COUNT_ZERO)
			{
				DisconnectProc(session);
				return true;
			}
		}
		else
		{
			DisconnectProc(session);
			return true;
		}

		/*InterlockedExchange8((char*)&session->isCompletingSend, true);

		bool ret = DecrementProc(session);
		int sendRet = 0;

		if (ret)
		{
			session->send.queue.Lock(false);
			session->send.queue.MoveFront(transferredSize);
			session->send.queue.Unlock(false);

			InterlockedExchange8((char*)&session->isSending, false);
			sendRet = SendPost(session);
		}
		else
		{
			DisconnectProc(session);
			return true;
		}

		if (sendRet == SEND_POST_IO_COUNT_ZERO)
		{
			DisconnectProc(session);
			return true;
		}

		if (sendRet != SEND_POST_PACKET_SENT)
		{
			InterlockedExchange8((char*)&session->isCompletingSend, false);
		}

		if (sendRet == SEND_POST_RING_QUEUE_EMPTY)
		{
			if (session->send.queue.GetUseSize() > 0 && session->isCompletingSend == false)
			{
				SendPost(session);
			}
		}*/
	}

	return true;
}

void CLanServerNoLock::CloseSessions()
{
}

void CLanServerNoLock::InitializeEmptyIndex()
{
	for (u_short i = mMaxClient; i > 0; --i)
	{
		mEmptyIndexes.push(i - 1);
	}

	InitializeSRWLock(&mStackLock);
}

u_int64 CLanServerNoLock::GenerateSessionID()
{
	if (mEmptyIndexes.empty())
	{
		return 0;
	}

	LockStack();
	u_short index = mEmptyIndexes.top();
	mEmptyIndexes.pop();
	UnlockStack();
	u_int64 id = index;

	id <<= (8 * 6);

	id |= mSessionIDCounter;
	mSessionIDCounter++;

	return id;
}

u_short CLanServerNoLock::GetIndexFromSessionNo(u_int64 sessionNo)
{
	return (u_short)(sessionNo >> (8 * 6));
}

void CLanServerNoLock::MonitorProc()
{
	if (mbMonitoring)
	{
		mSessionPool->Lock(true);
		int poolSize = mSessionPool->GetSize();
		int poolCapa = mSessionPool->GetCapacity();
		mSessionPool->Unlock(true);

		wprintf_s(L"\nMonitoring[M]: (%d) | Quit[Q]\n", mbMonitoring);
		wprintf_s(L"ZeroCopy[Z]: (%d) | Nagle[N]: (%d)\n", mbZeroCopy, mbNagle);
		wprintf_s(L"=======================================\n[Total Accept Count: %u]\n[Total Diconnect Count: %u]\n[Live Session Count: %u]\n",
			mMonitor.acceptCount,
			mMonitor.disconnectCount,
			mMonitor.acceptCount - mMonitor.disconnectCount);
		wprintf_s(L"=======================================\n[Send TPS: %u]\n[Recv TPS: %u]\n[Pool Usage: (%d / %d)]\n=======================================\n",
			mMonitor.sendTPS,
			mMonitor.recvTPS,
			poolSize,
			poolCapa);
	}

	//MonitorLock();

	mMonitor.recvTPS = 0;
	mMonitor.sendTPS = 0;

	//MonitorUnlock();
}

void CLanServerNoLock::LockStack()
{
	AcquireSRWLockExclusive(&mStackLock);
}

void CLanServerNoLock::UnlockStack()
{
	ReleaseSRWLockExclusive(&mStackLock);
}

CLanServerNoLock::~CLanServerNoLock()
{
	closesocket(mListenSocket);
	if (mSessionPool != nullptr)
	{
		delete mSessionPool;
	}
	if (mhThreads != nullptr)
	{
		delete[] mhThreads;
	}
	if (mSessionArray != nullptr)
	{
		delete[] mSessionArray;
	}
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Lib End\n");
}

bool CLanServerNoLock::Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
	if (mbIsRunning == true)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running\n");
		return false;
	}

	procademy::CCrashDump::CCrashDump();

	mPort = port;
	mBindIP = ip;
	mWorkerThreadSize = createThread;
	mMaxRunThreadSize = runThread;
	mMaxClient = maxClient;
	mbNagle = nagle;
	mSessionPool = new procademy::ObjectPool<Session>(10);

	mhThreads = new HANDLE[createThread + 1];
	mSessionArray = new Session * [maxClient];
	ZeroMemory(mSessionArray, maxClient * sizeof(Session*));

	if (!CreateListenSocket())
	{
		return false;
	}

	BeginThreads();

	mbIsRunning = true;

	InitializeEmptyIndex();

	return true;
}

bool CLanServerNoLock::Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
	return Start(port, INADDR_ANY, createThread, runThread, nagle, maxClient);
}

void CLanServerNoLock::Stop()
{
}

int CLanServerNoLock::GetSessionCount()
{
	return 0;
}

void CLanServerNoLock::WaitForThreadsFin()
{
	HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

	while (1)
	{
		DWORD retval = WaitForSingleObject(dummyEvent, 1000);

		if (retval == WAIT_TIMEOUT)
		{
			MonitorProc();

			if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x5A) & 0x8001) // Z
			{
				if (mbZeroCopy)
				{
					mbZeroCopy = false;
					wprintf_s(L"\nUnset ZeroCopy Mode\n\n");
				}
				else
				{
					mbZeroCopy = true;
					wprintf_s(L"\nSet ZeroCopy Mode\n\n");
				}
			}

			if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x4D) & 0x8001) // M
			{
				if (mbMonitoring)
				{
					mbMonitoring = false;
					wprintf_s(L"\nUnset monitoring Mode\n\n");
				}
				else
				{
					mbMonitoring = true;
					wprintf_s(L"\nSet monitoring Mode\n\n");
				}
			}

			if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x50) & 0x8001) // P
			{
				wprintf_s(L"========================\n");
				for (u_short i = 0; i < mMaxClient; ++i)
				{
					if (mSessionArray[i] != nullptr)
					{
						int recvSize = mSessionArray[i]->recv.queue.GetUseSize();
						int sendSize = mSessionArray[i]->send.queue.GetUseSize();

						if (recvSize > 0 || sendSize > 0)
						{
							wprintf_s(L"[Socket: %llu] [RecvUse: %d] [SendUse: %d] [Sending: %d] [io_Count: %d] [Alive: %d]\n",
								mSessionArray[i]->socket, recvSize, sendSize, mSessionArray[i]->isSending, mSessionArray[i]->ioCount,
								mSessionArray[i]->bIsAlive);
						}
					}
				}
			}

			if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x51) & 0x8001) // Q
			{
				wprintf_s(L"Exit\n");

				PostQueuedCompletionStatus(mHcp, 0, 0, 0);

				closesocket(mListenSocket);

				break;
			}
		}
	}

	CloseHandle(dummyEvent);

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

bool CLanServerNoLock::Disconnect(SESSION_ID SessionID)
{
	return false;
}

int CLanServerNoLock::SendPacket(SESSION_ID SessionID, CPacket* packet)
{
	Session* session = FindSession(SessionID);
	int ret = 0;

	/*st_NETWORK_HEADER header;

	header.byCode = dfNETWORK_CODE;
	header.wPayloadSize = packet->GetSize();*/

	session->send.queue.Lock(false);
	//session->send.queue.Enqueue((char*)&header, sizeof(header));
	session->send.queue.Enqueue(packet->GetBufferPtr(), packet->GetSize());
	session->send.queue.Unlock(false);

	ret = SendPost(session);

	if (ret == SEND_POST_IO_COUNT_ZERO)
	{
		DisconnectProc(session);
	}

	return ret;
}
