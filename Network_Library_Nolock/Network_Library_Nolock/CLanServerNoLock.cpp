#include "CLanServerNoLock.h"
#include "CLogger.h"
#include "CPacket.h"
#include "CCrashDump.h"
#include "CDebugger.h"

Session* CLanServerNoLock::FindSession(u_int64 sessionNo)
{
	u_short index = GetIndexFromSessionNo(sessionNo);

	return mSessionArray + index;
}

void CLanServerNoLock::InsertSessionData(Session* session)
{
	u_short index = GetIndexFromSessionNo(session->sessionID);

	mSessionArray[index] = *session;
}

void CLanServerNoLock::DeleteSessionData(u_int64 sessionNo)
{
	u_short index = GetIndexFromSessionNo(sessionNo);

	mEmptyIndexes.Push(index);
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

	int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recvOverlapped, nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		DecrementIOProc(session);

		return false;
	}

	return true;
}

void CLanServerNoLock::SendPost(Session* session)
{
	WSABUF buffers[100];

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

		int sendRet = WSASend(session->socket, buffers, session->numSendingPacket, nullptr, 0, &session->sendOverlapped, nullptr);

		if (sendRet == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err == WSA_IO_PENDING)
			{
				break;
			}

			DecrementIOProc(session);
		}

		break;
	} while (1);
}

void CLanServerNoLock::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
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
		CPacket* packetBufs[100];
		DWORD snapSize = session->sendQ.Peek(packetBufs);

		for (DWORD i = 0; i < snapSize; ++i)
		{
			bufs[i].buf = packetBufs[i]->GetBufferPtr();
			bufs[i].len = packetBufs[i]->GetSize();
		}

		session->numSendingPacket = snapSize;

		/*char* pRear = session->send.queue.GetRearBufferPtr();
		char* pFront = session->send.queue.GetFrontBufferPtr();
		char* pBuf = session->send.queue.GetBuffer();
		int capa = session->send.queue.GetCapacity();

		if (pRear >= pFront)
		{
			bufs[0].buf = pFront;
			bufs[0].len = (ULONG)(pRear - pFront);
			bufs[1].buf = pRear;
			bufs[1].len = 0;
		}
		else
		{
			bufs[0].buf = pFront;
			bufs[0].len = (ULONG)(capa + 1 - (pFront - pBuf));
			bufs[1].buf = pBuf;
			bufs[1].len = (ULONG)(pRear - pBuf);
		}*/

		/*if (pFront - pBuf < 2997)
		{
			USHORT num = *((USHORT*)&(*(pFront + 2)));

			session->debugs[session->index].begin = num;
		}
		else
		{
			session->debugs[session->index].begin = 0;
		}
		if ((int)(pRear - pBuf) - 8 >= 0)
		{
			USHORT num2 = *((USHORT*)&(*(pRear - 8)));

			session->debugs[session->index].end = num2;
		}
		else
		{
			session->debugs[session->index].end = 0;
		}

		if (session->debugs[session->index].begin != 0 && 
			session->debugs[(unsigned char)(session->index - 1)].end != 0 &&
			session->debugs[(unsigned char)(session->index - 1)].end >= session->debugs[session->index].begin &&
			session->index > 1)
		{
			int test = 0;
		}

		session->index++;*/
	}
}

void CLanServerNoLock::IncrementIOProc(Session* session)
{
	InterlockedIncrement(&session->ioBlock.ioCount);
}

void CLanServerNoLock::DecrementIOProc(Session* session)
{
	SessionIoCount ret;
	ret.ioCount = InterlockedDecrement(&session->ioBlock.ioCount);

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

void CLanServerNoLock::ReleaseProc(Session* session)
{
	SessionIoCount released;
	released.releaseCount.isReleased = 1;

	if (InterlockedCompareExchange(&session->ioBlock.ioCount, released.ioCount, 0) != 0)
	{
		return;
	}

	u_int64 id = session->sessionID;
	InterlockedIncrement16((short*)&mMonitor.disconnectCount);
	session->bIsAlive = false;

	OnClientLeave(id);

	closesocket(session->socket);

	//LockSessionMap();
	DeleteSessionData(id);
	//UnlockSessionMap();
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

	InterlockedIncrement(&session->ioBlock.ioCount);
	InterlockedDecrement16(&session->ioBlock.releaseCount.isReleased);
	OnClientJoin(session->sessionID);
	RecvPost(session);

	return true;
}

Session* CLanServerNoLock::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
{
	u_int64 id = GenerateSessionID();

	if (id == 0)
	{
		return nullptr;
	}

	//Session* session = mSessionPool->Alloc();
	Session* session = FindSession(id);
	session->socket = client;
	session->ip = clientAddr.sin_addr.S_un.S_addr;
	session->port = clientAddr.sin_port;
	session->isSending = false;
	session->sessionID = id;
	session->sendQ.Clear();
	session->recvQ.ClearBuffer();
	session->recvQ.InitializeLock();
	session->bIsAlive = true;

	ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));
	InitializeSRWLock(&session->lock);

	HANDLE hResult = CreateIoCompletionPort((HANDLE)client, mHcp, (ULONG_PTR)session, 0);

	if (hResult == NULL)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]\n", WSAGetLastError());
		closesocket(mListenSocket);

		return nullptr;
	}

	//LockSessionMap();
	//InsertSessionData(session);
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

	if (transferredSize == 0) // Normal close
	{
		DecrementIOProc(session);

		return true;
	}

	if (pOverlapped == &session->recvOverlapped) // Recv
	{
		CompleteRecv(session, transferredSize);
	}
	else // Send
	{
		CompleteSend(session, transferredSize);
	}

	return true;
}

void CLanServerNoLock::CompleteRecv(Session* session, DWORD transferredSize)
{
	session->recvQ.MoveRear(transferredSize);

	DWORD count = 0;

	while (count < transferredSize)
	{
		CPacket* packet = new CPacket;
		//MonitorLock();
		InterlockedIncrement(&mMonitor.recvTPS);
		InterlockedIncrement(&mMonitor.sendTPS);
		//MonitorUnlock();
		//session->recv.queue.MoveRear(10);
		int ret = session->recvQ.Dequeue(packet->GetFrontPtr(), 10);

		//if (ret != 10)
		//{
		//	wprintf_s(L"Error\n");
		//}

		/*int num = *(packet.GetFrontPtr() + 2);

		CLogger::_Log(dfLOG_LEVEL_DEBUG, L"%d\n", num);*/

		packet->MoveRear(ret);

		OnRecv(session->sessionID, packet); // -> SendPacket

		/*if (count == 0)
		{
			CDebugger::_Log(L"PacketProc Begin [S %5d] [L %4d]", session->sessionID, session->lastNum);
		}*/
		//count += (sizeof(header) + header.wPayloadSize);
		count += ret;
	}

	RecvPost(session);
}

void CLanServerNoLock::CompleteSend(Session* session, DWORD transferredSize)
{
	CPacket* packet;

	for (int i = 0; i < session->numSendingPacket; ++i)
	{
		session->sendQ.Dequeue(&packet);
		
		delete packet;
		packet = nullptr;
	}

	//session->numSendingPacket = 0;

	InterlockedExchange8((char*)&session->isSending, false);

	SendPost(session);
}

void CLanServerNoLock::CloseSessions()
{
}

void CLanServerNoLock::InitializeEmptyIndex()
{
	for (u_short i = mMaxClient; i > 0; --i)
	{
		mEmptyIndexes.Push(i - 1);
	}

	//InitializeSRWLock(&mStackLock);
}

u_int64 CLanServerNoLock::GenerateSessionID()
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

u_short CLanServerNoLock::GetIndexFromSessionNo(u_int64 sessionNo)
{
	return (u_short)(sessionNo >> (8 * 6));
}

void CLanServerNoLock::MonitorProc()
{
	if (mbMonitoring)
	{
		wprintf_s(L"\nMonitoring[M]: (%d) | Quit[Q]\n", mbMonitoring);
		wprintf_s(L"ZeroCopy[Z]: (%d) | Nagle[N]: (%d)\n", mbZeroCopy, mbNagle);
		wprintf_s(L"=======================================\n[Total Accept Count: %u]\n[Total Diconnect Count: %u]\n[Live Session Count: %u]\n",
			mMonitor.acceptCount,
			mMonitor.disconnectCount,
			mMonitor.acceptCount - mMonitor.disconnectCount);
		wprintf_s(L"=======================================\n[Send TPS: %u]\n[Recv TPS: %u]\n=======================================\n",
			mMonitor.sendTPS,
			mMonitor.recvTPS);
	}

	//MonitorLock();

	mMonitor.recvTPS = 0;
	mMonitor.sendTPS = 0;

	//MonitorUnlock();
}

CLanServerNoLock::~CLanServerNoLock()
{
	closesocket(mListenSocket);
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

	mPort = port;
	mBindIP = ip;
	mWorkerThreadSize = createThread;
	mMaxRunThreadSize = runThread;
	mMaxClient = maxClient;
	mbNagle = nagle;

	mhThreads = new HANDLE[(long long)createThread + 1];
	mSessionArray = new Session[maxClient];

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
					if (mSessionArray[i].bIsAlive != true)
					{
						int recvSize = mSessionArray[i].recvQ.GetUseSize();
						int sendSize = mSessionArray[i].sendQ.GetSize();

						if (recvSize > 0 || sendSize > 0)
						{
							wprintf_s(L"[Socket: %llu] [RecvUse: %d] [SendUse: %d] [Sending: %d] [io_Count: %d] [Alive: %d]\n",
								mSessionArray[i].socket, recvSize, sendSize, mSessionArray[i].isSending, mSessionArray[i].ioBlock.ioCount,
								mSessionArray[i].bIsAlive);
						}
					}
				}
			}

			if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x44) & 0x8001) // D
			{
				CRASH();
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

	IncrementIOProc(session);

	if (SessionID != session->sessionID)
	{
		DecrementIOProc(session);
	}

	/*st_NETWORK_HEADER header;

	header.byCode = dfNETWORK_CODE;
	header.wPayloadSize = packet->GetSize();*/
	session->sendQ.Enqueue(packet);

	SendPost(session);

	DecrementIOProc(session);

	return ret;
}
