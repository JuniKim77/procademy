#include "CMMOServer.h"
#include "CLogger.h"
#include "TextParser.h"
#include "CNetPacket.h"
#include "CFrameSkipper.h"

procademy::CMMOServer::CMMOServer()
{
	LoadInitFile(L"Server.cnf");
	Init();
	BeginThreads();
}

procademy::CMMOServer::~CMMOServer()
{
	closesocket(mListenSocket);
	if (mhThreads != nullptr)
	{
		delete[] mhThreads;
	}

	if (mSessionArray != nullptr)
	{
		for (int i = 0; i < mMaxClient; ++i)
		{
			delete mSessionArray[i];
		}

		delete[] mSessionArray;
	}
	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CMMOServer End");
}

bool procademy::CMMOServer::Start()
{
	if (mbBegin == true)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running");
		return false;
	}

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CMMOServer Begin");

	if (!CreateListenSocket())
	{
		return false;
	}

	SetEvent(mBeginEvent);

	return true;
}

void procademy::CMMOServer::Stop()
{
	mbBegin = false;
	BOOL ret;

	closesocket(mListenSocket);

	for (USHORT i = 0; i < mMaxClient; ++i)
	{
		ret = CancelIoEx((HANDLE)mSessionArray[i]->socket, nullptr);
		//CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Cancel Ret: %d, Err: %d", ret, GetLastError());
	}

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Stop CMMOServer");
}

void procademy::CMMOServer::LoadInitFile(const WCHAR* fileName)
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

void procademy::CMMOServer::QuitServer()
{
	mbExit = true;

	Stop();

	PostQueuedCompletionStatus(mIOCP, 0, 0, 0);

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

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Quit CMMOServer");
}

void procademy::CMMOServer::SetZeroCopy(bool on)
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

void procademy::CMMOServer::SetNagle(bool on)
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

unsigned int __stdcall procademy::CMMOServer::MonitorThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	server->MonitorProc();

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::AcceptThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	while (!server->mbExit)
	{
		if (server->mbBegin)
		{
			server->AcceptProc();
		}
		else
		{
			WaitForSingleObject(server->mBeginEvent, INFINITE);
			SetEvent(server->mBeginEvent);
			server->mbBegin = true;
		}
	}

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::IocpWorkerThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	while (!server->mbExit)
	{
		server->GQCSProc();
	}

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::GameThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	while (!server->mbExit)
	{
		if (server->mbBegin)
		{
			server->GameThreadProc();
		}
		else
		{
			WaitForSingleObject(server->mBeginEvent, INFINITE);
			SetEvent(server->mBeginEvent);
			server->mbBegin = true;
		}
	}

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::AuthThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	while (!server->mbExit)
	{
		if (server->mbBegin)
		{
			server->AuthThreadProc();
		}
		else
		{
			WaitForSingleObject(server->mBeginEvent, INFINITE);
			SetEvent(server->mBeginEvent);
			server->mbBegin = true;
		}
	}

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::SendThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	while (!server->mbExit)
	{
		if (server->mbBegin)
		{
			server->SendThreadProc();
		}
		else
		{
			WaitForSingleObject(server->mBeginEvent, INFINITE);
			SetEvent(server->mBeginEvent);
			server->mbBegin = true;
		}
	}

	return 0;
}

void procademy::CMMOServer::Init()
{
	WORD		version = MAKEWORD(2, 2);
	WSADATA		data;

	WSAStartup(version, &data);
	CLogger::SetDirectory(L"_log");
	mBeginEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);

	mhThreads = new HANDLE[(long long)mWorkerThreadNum + 5];
	mSessionArray = (CSession**)(new CSession*[mMaxClient]);

	InitializeEmptyIndex();
	CreateIOCP();
}

bool procademy::CMMOServer::CreateIOCP()
{
	// 논리 코어 개수 확인 로직
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	if (mActiveThreadNum > si.dwNumberOfProcessors)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Setting: Max Running thread is larger than the number of processors");
	}

	mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, mActiveThreadNum);

	if (mIOCP == NULL)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());
		closesocket(mListenSocket);
		return false;
	}

	return true;
}

bool procademy::CMMOServer::CreateListenSocket()
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

bool procademy::CMMOServer::BeginThreads()
{
	BYTE i = 0;

	mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, AcceptThread, this, 0, nullptr);

	for (; i <= mWorkerThreadNum; ++i)
	{
		mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, IocpWorkerThread, this, 0, nullptr);
	}

	mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);
	mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, GameThread, this, 0, nullptr);
	mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, AuthThread, this, 0, nullptr);
	mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, SendThread, this, 0, nullptr);

	mNumThreads = i;

	return true;
}

void procademy::CMMOServer::InitializeEmptyIndex()
{
	for (u_short i = mMaxClient; i > 0; --i)
	{
		mEmptyIndexes.Push(i - 1);
	}
}

void procademy::CMMOServer::AcceptProc()
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

	CreateSession(client, clientAddr);
}

void procademy::CMMOServer::MonitorProc()
{
}

void procademy::CMMOServer::GQCSProc()
{
	while (1)
	{
		DWORD transferredSize = 0;
		CSession* completionKey = nullptr;
		WSAOVERLAPPED* pOverlapped = nullptr;
		CSession* session = nullptr;

		BOOL gqcsRet = GetQueuedCompletionStatus(mIOCP, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);
#ifdef PROFILE
		CProfiler::Begin(L"GQCS_Net");
#endif // PROFILE

		// ECHO Server End
		if (transferredSize == 0 && (PULONG_PTR)completionKey == nullptr && pOverlapped == nullptr)
		{
			PostQueuedCompletionStatus(mIOCP, 0, 0, 0);

			return;
		}

		if (pOverlapped == nullptr) // I/O Fail
		{
			OnError(10000, L"IOCP Error");

			return;
		}

		session = (CSession*)completionKey;

		if (transferredSize != 0)
		{
			if (pOverlapped == &session->recvOverlapped) // Recv
			{
#ifdef PROFILE
				//CProfiler::Begin(L"CompleteRecv");
				CompleteRecv(session, transferredSize);
				//CProfiler::End(L"CompleteRecv");
#else
				CompleteRecv(session, transferredSize);
#endif // PROFILE
			}
			else // Send
			{
#ifdef PROFILE
				//CProfiler::Begin(L"CompleteSend");
				CompleteSend(session, transferredSize);
				//CProfiler::End(L"CompleteSend");
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

void procademy::CMMOServer::GameThreadProc()
{
	CFrameSkipper skipper;
	skipper.SetMaxFrame(100);
	skipper.Reset();

	while (mbBegin)
	{
		GameLoopProc();

		skipper.CheckTime();

		if (skipper.IsOverSecond())
		{
			skipper.Refresh();
			// 로깅
		}

		skipper.RunSleep();
	}
}

void procademy::CMMOServer::AuthThreadProc()
{
	CFrameSkipper skipper;
	skipper.SetMaxFrame(100);
	skipper.Reset();

	while (mbBegin)
	{
		AuthLoopProc();

		skipper.CheckTime();

		if (skipper.IsOverSecond())
		{
			skipper.Refresh();
			// 로깅
		}

		skipper.RunSleep();
	}
}

void procademy::CMMOServer::SendThreadProc()
{
	CFrameSkipper skipper;
	skipper.SetMaxFrame(100);
	skipper.Reset();

	while (mbBegin)
	{
		SendLoopProc();

		skipper.CheckTime();

		if (skipper.IsOverSecond())
		{
			skipper.Refresh();
			// 로깅
		}

		skipper.RunSleep();
	}
}

void procademy::CMMOServer::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
{
	if (mEmptyIndexes.IsEmpty())
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Session Index Full\n");

		CRASH();
	}

	u_short index = 0;
	mEmptyIndexes.Pop(&index);

	CSession* session = mSessionArray[index];

	session->socket = client;
	session->ip = clientAddr.sin_addr.S_un.S_addr;
	session->port = clientAddr.sin_port;
	session->isSending = false;
	session->sessionID = mSessionIDCounter++;
	session->status = CSession::en_AUTH_READY;

	ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
}

void procademy::CMMOServer::IncrementIOProc(CSession* session, int logic)
{
	InterlockedIncrement(&session->ioBlock.ioCount);
}

void procademy::CMMOServer::DecrementIOProc(CSession* session, int logic)
{
	InterlockedDecrement(&session->ioBlock.ioCount);
}

void procademy::CMMOServer::CompleteRecv(CSession* session, DWORD transferredSize)
{
	session->recvQ.MoveRear(transferredSize);
	CNetPacket::st_Header header;
	DWORD count = 0;
	bool status = true;

	while (count < transferredSize)
	{
		if (session->recvQ.GetUseSize() <= CNetPacket::HEADER_MAX_SIZE)
			break;

		session->recvQ.Peek((char*)&header, CNetPacket::HEADER_MAX_SIZE);

		if (header.code != CNetPacket::sCode)
		{
			status = false;
			break;
		}

		if (header.len > CNetPacket::eBUFFER_DEFAULT)
		{
			status = false;
			break;
		}

		if (session->recvQ.GetUseSize() < (CNetPacket::HEADER_MAX_SIZE + header.len))
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
		packet->AddRef();
		if (session->recvCompleteQ.Enqueue(packet) == false)
		{
			CRASH();
		}

		count += (ret + sizeof(SHORT));
		packet->SubRef();
	}
}

void procademy::CMMOServer::CompleteSend(CSession* session, DWORD transferredSize)
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
}

bool procademy::CMMOServer::RecvPost(CSession* session, bool isFirst)
{
	if (isFirst == false)
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

bool procademy::CMMOServer::SendPost(CSession* session)
{
	WSABUF buffers[100];

	if (session->isSending)
	{
		return false;
	}

	session->isSending = true;

	SetWSABuf(buffers, session, false);

	ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));

	IncrementIOProc(session, 30000);

	int sendRet = WSASend(session->socket, buffers, session->numSendingPacket, nullptr, 0, &session->sendOverlapped, nullptr);

	if (sendRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		DecrementIOProc(session, 20050);

		return false;
	}

	return true;
}

void procademy::CMMOServer::SendPacket(CSession* session, CNetPacket* packet)
{
	u_int64 sessionID = session->sessionID;

	IncrementIOProc(session, 20000);

	if (session->ioBlock.releaseCount.isReleased == 1 || session->sessionID != sessionID)
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

	DecrementIOProc(session, 20020);
}

void procademy::CMMOServer::SetWSABuf(WSABUF* bufs, CSession* session, bool isRecv)
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

void procademy::CMMOServer::ReleaseProc(CSession* session)
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
	InterlockedIncrement((LONG*)&disconnectCount);

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
}

void procademy::CMMOServer::AuthLoopProc()
{
	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::en_AUTH_READY:
		{
			AuthReadySessionProc(session);
			break;
		}
		case CSession::en_AUTH_RUN:
		{
			AuthCompleteRecvProc(session);
			
			session->OnAuth_Update();

			AuthSessionToReleaseProc(session);
			break;
		}
		case CSession::en_AUTH_RELEASE:
		{
			if (session->isSending == false)
			{
				AuthReleaseProc(session);
			}
			break;
		}
		default:
			break;
		}
	}
}

void procademy::CMMOServer::AuthReadySessionProc(CSession* session)
{
	// 세션 최초 recvPost 호출
	IncrementIOProc(session, 10000);
	session->ioBlock.releaseCount.isReleased = 0;

	session->OnAuth_ClientJoin();

	RecvPost(session, true);
}

void procademy::CMMOServer::AuthCompleteRecvProc(CSession* session)
{
	int count = 0;
	CNetPacket* packet = nullptr;

	while (count < mAuthPacketLoopNum)
	{
		if (session->recvCompleteQ.IsEmpty())
		{
			break;
		}

		packet = session->recvCompleteQ.Dequeue();

		session->OnAuth_Packet(packet);
		count++;
	}
}

void procademy::CMMOServer::AuthSessionToReleaseProc(CSession* session)
{
	if (session->sessionEnd)
	{
		session->status = CSession::en_AUTH_RELEASE;
	}
}

void procademy::CMMOServer::AuthReleaseProc(CSession* session)
{
	session->OnAuth_ClientRelease();
	session->OnAuth_ClientLeave();
	session->status = CSession::en_GAME_READY;
}

void procademy::CMMOServer::SendLoopProc()
{
	int count = 0;

	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::en_AUTH_RUN:
		case CSession::en_GAME_RUN:
		{
			SendPacketProc(session);
		}
		default:
			break;
		}
	}
}

void procademy::CMMOServer::SendPacketProc(CSession* session)
{
	if (session->sendQ.IsEmpty() == false)
	{
		SendPost(session);
	}
}

void procademy::CMMOServer::GameLoopProc()
{
	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::en_GAME_READY:
		{
			GameReadySessionProc(session);
			break;
		}
		case CSession::en_GAME_RUN:
		{
			GameCompleteRecvProc(session);

			session->OnAuth_Update();

			GameSessionToReleaseProc(session);
			break;
		}
		case CSession::en_GAME_RELEASE:
		{
			if (session->isSending == false)
			{
				GameReleaseProc(session);
			}
			break;
		}
		default:
			break;
		}
	}
}

void procademy::CMMOServer::GameReadySessionProc(CSession* session)
{
	session->OnGame_ClientJoin();

	// 할게 있나...?

	session->status = CSession::en_GAME_RUN;
}

void procademy::CMMOServer::GameCompleteRecvProc(CSession* session)
{
	int count = 0;
	CNetPacket* packet = nullptr;

	while (count < mGamePacketLoopNum)
	{
		if (session->recvCompleteQ.IsEmpty())
		{
			break;
		}

		packet = session->recvCompleteQ.Dequeue();

		session->OnGame_Packet(packet);
		count++;
	}
}

void procademy::CMMOServer::GameSessionToReleaseProc(CSession* session)
{
	if (session->sessionEnd)
	{
		session->status = CSession::en_GAME_RELEASE;
	}
}

void procademy::CMMOServer::GameReleaseProc(CSession* session)
{
	session->OnGame_ClientRelease();
	session->OnGame_ClientLeave();
	session->status = CSession::en_NONE_USE;
}


