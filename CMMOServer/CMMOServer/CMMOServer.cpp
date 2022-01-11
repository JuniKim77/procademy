#pragma warning(disable:6387)

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

	SetEvent(mAcceptEvent);
	SetEvent(mGameEvent);
	SetEvent(mAuthEvent);
	SetEvent(mSendEvent);

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
	BYTE        code;
	BYTE        key;

	tp.LoadFile(fileName);

	tp.GetValue(L"BIND_IP", mBindIP);

	tp.GetValue(L"BIND_PORT", &num);
	mPort = (u_short)num;

	tp.GetValue(L"PACKET_CODE", &num);
	code = (BYTE)num;
	CNetPacket::SetCode(code);

	tp.GetValue(L"PACKET_KEY", &num);
	key = (BYTE)num;
	CNetPacket::SetPacketKey(key);

	tp.GetValue(L"IOCP_WORKER_THREAD", &num);
	mWorkerThreadNum = (BYTE)num;

	tp.GetValue(L"IOCP_ACTIVE_THREAD", &num);
	mActiveThreadNum = (BYTE)num;

	tp.GetValue(L"CLIENT_MAX", &num);
	mMaxClient = (u_short)num;

	tp.GetValue(L"AUTH_MAX_TRANSFER", &mMaxTransferToAuth);
	tp.GetValue(L"GAME_MAX_TRANSFER", &mMaxTransferToGame);

	tp.GetValue(L"NAGLE", buffer);
	if (wcscmp(L"TRUE", buffer) == 0)
		mbNagle = true;
	else
		mbNagle = false;

	tp.GetValue(L"ZERO_COPY", buffer);
	if (wcscmp(L"TRUE", buffer) == 0)
		mbZeroCopy = true;
	else
		mbZeroCopy = false;

	tp.GetValue(L"TIMEOUT_DISCONNECT", &mTimeOut);

	tp.GetValue(L"POOL_SIZE_CHECK", buffer);
	if (wcscmp(L"TRUE", buffer) == 0)
		CNetPacket::sPacketPool.OnOffCounting();

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

	SetEvent(mAcceptEvent);

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

void procademy::CMMOServer::SetSession(CSession* session)
{
	mSessionArray[mSessionCount++] = session;
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
			WaitForSingleObject(server->mAcceptEvent, INFINITE);
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
			WaitForSingleObject(server->mGameEvent, INFINITE);
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
			WaitForSingleObject(server->mAuthEvent, INFINITE);
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
			WaitForSingleObject(server->mSendEvent, INFINITE);
			server->mbBegin = true;
		}
	}

	return 0;
}

void procademy::CMMOServer::Init()
{
	WORD		version = MAKEWORD(2, 2);
	WSADATA		data;

	int ret = WSAStartup(version, &data);
	CLogger::SetDirectory(L"_log");
	mAcceptEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);
	mGameEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);
	mAuthEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);
	mSendEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);

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
	HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

	while (!mbExit)
	{
		DWORD retval = WaitForSingleObject(dummyEvent, 1000);

		if (retval == WAIT_TIMEOUT)
		{
			mMonitor.prevRecvTPS = recvTPS;
			mMonitor.prevSendTPS = maSendTPS;
			mMonitor.acceptTotal = acceptTotal;
			mMonitor.acceptTPS = acceptTPS;
			mMonitor.prevSendLoopCount = sendLoopCount;
			mMonitor.prevAuthLoopCount = authLoopCount;
			mMonitor.prevGameLoopCount = gameLoopCount;

			sendLoopCount = 0;
			authLoopCount = 0;
			gameLoopCount = 0;
			recvTPS = 0;
			maSendTPS = 0;
			acceptTPS = 0;
		}
	}

	CloseHandle(dummyEvent);
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
		gameLoopCount++;

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
		authLoopCount++;

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
		sendLoopCount++;

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
	session->index = index;
	session->lastRecvTime = GetTickCount64();
	
	ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));

	HANDLE hResult = CreateIoCompletionPort((HANDLE)client, mIOCP, (ULONG_PTR)session, 0);

	if (hResult == NULL)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateSession [Error: %d]", WSAGetLastError());
		closesocket(mListenSocket);
	}

	acceptTotal++;
	acceptTPS++;
	InterlockedIncrement(&joinCount);

	session->status = CSession::SESSION_STATUS::en_AUTH_READY;
}

void procademy::CMMOServer::IncrementIOProc(CSession* session, int logic)
{
	InterlockedIncrement(&session->ioCount);
}

void procademy::CMMOServer::DecrementIOProc(CSession* session, int logic)
{
	long ret = InterlockedDecrement(&session->ioCount);

	if (ret < 0)
		CRASH();

	if (ret == 0)
	{
		session->sessionEnd = true;
		session->isSending = false;
	}
}

void procademy::CMMOServer::CompleteRecv(CSession* session, DWORD transferredSize)
{
	session->recvQ.MoveRear(transferredSize);
	CNetPacket::st_Header header;
	DWORD count = 0;
	bool status = true;

	while (count < transferredSize)
	{
		int useSize = session->recvQ.GetUseSize();

		if (useSize <= CNetPacket::HEADER_MAX_SIZE)
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
		packet->AddRef();
		if (session->recvCompleteQ.Enqueue(packet) == false)
		{
			CRASH();
		}

		count += (ret + CNetPacket::HEADER_MAX_SIZE);
		packet->SubRef();
	}

	if (status)
	{
		RecvPost(session);
	}
}

void procademy::CMMOServer::CompleteSend(CSession* session, DWORD transferredSize)
{
	CNetPacket* packet;
	InterlockedAdd((LONG*)&maSendTPS, session->numSendingPacket);
	for (int i = 0; i < session->numSendingPacket; ++i)
	{
		packet = session->sendQ.Dequeue();

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

	ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));

	int recvRet = WSARecv(session->socket, buffers, 2, nullptr, &flags, &session->recvOverlapped, nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSA_IO_PENDING)
		{
			return true;
		}

		if (err != WSAECONNRESET && err != WSAEINTR)
		{
			CRASH();
		}

		DecrementIOProc(session, 10050);

		return false;
	}

	return true;
}

bool procademy::CMMOServer::SendPost(CSession* session)
{
	WSABUF buffers[200];

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

		if (err != WSAECONNRESET && err != WSAEINTR && err != WSAECONNABORTED)
		{
			CRASH();
		}

		if (err == WSAECONNABORTED)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"SendPost Unusual Error %d", err);
		}

		DecrementIOProc(session, 20050);

		session->isSending = false;

		return false;
	}

	return true;
}

void procademy::CMMOServer::SendPacket(CSession* session, CNetPacket* packet)
{
	u_int64 sessionID = session->sessionID;

	packet->AddRef();
	session->sendQ.Enqueue(packet);
}

void procademy::CMMOServer::SetWSABuf(WSABUF* bufs, CSession* session, bool isRecv)
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
		CNetPacket* packetBufs[200];

		DWORD snapSize = session->sendQ.Peek(packetBufs, 200);

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
	CNetPacket* dummy;

	InterlockedDecrement(&joinCount);

	closesocket(session->socket);

	session->toGame = false;
	session->sessionEnd = false;
	session->status = CSession::SESSION_STATUS::en_NONE_USE;

	while (1)
	{
		if ((dummy = session->sendQ.Dequeue()) == nullptr)
		{
			break;
		}

		dummy->SubRef();
	}
	session->recvQ.ClearBuffer();

	while (1)
	{
		dummy = session->recvCompleteQ.Dequeue();

		if (dummy == nullptr)
		{
			break;
		}

		dummy->SubRef();
	}

	ZeroMemory(&session->sendOverlapped, sizeof(WSAOVERLAPPED));
	ZeroMemory(&session->recvOverlapped, sizeof(WSAOVERLAPPED));

	mEmptyIndexes.Push(session->index);
}

void procademy::CMMOServer::Disconnect(CSession* session)
{
	IncrementIOProc(session, 40000);

	BOOL ret = CancelIoEx((HANDLE)session->socket, nullptr);

	DecrementIOProc(session, 40040);
}

void procademy::CMMOServer::AuthLoopProc()
{
	int transferCount = 0;

	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::SESSION_STATUS::en_AUTH_READY:
		{
			if (transferCount < mMaxTransferToAuth)
			{
				AuthReadySessionProc(session);
				transferCount++;
			}
			break;
		}
		case CSession::SESSION_STATUS::en_AUTH_RUN:
		{
			AuthCompleteRecvProc(session);
			AuthTimeoutProc(session);
			AuthSessionToReleaseProc(session);
			AuthSessionToGameProc(session);
			break;
		}
		case CSession::SESSION_STATUS::en_AUTH_RELEASE:
		{
			AuthReleaseProc(session);
			break;
		}
		default:
			break;
		}
	}

	OnAuth_Update();
}

void procademy::CMMOServer::AuthReadySessionProc(CSession* session)
{
	// 세션 최초 recvPost 호출
	IncrementIOProc(session, 10000);

	session->sessionEnd = false;

	session->OnAuth_ClientJoin();

	RecvPost(session, true);

	session->status = CSession::SESSION_STATUS::en_AUTH_RUN;

	mAuthPlayerNum++;
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

		packet->SubRef();
	}

	if (count > 0)
	{
		session->lastRecvTime = GetTickCount64();
	}
}

void procademy::CMMOServer::AuthSessionToReleaseProc(CSession* session)
{
	if (session->sessionEnd)
	{
		session->toGame = false;
		session->status = CSession::SESSION_STATUS::en_AUTH_RELEASE_REQ;
	}
}

void procademy::CMMOServer::AuthSessionToGameProc(CSession* session)
{
	if (session->toGame)
	{
		session->OnAuth_ClientLeave();
		session->status = CSession::SESSION_STATUS::en_GAME_READY;

		mAuthPlayerNum--;
	}
}

void procademy::CMMOServer::AuthReleaseProc(CSession* session)
{
	session->OnAuth_ClientLeave();
	session->OnAuth_ClientRelease();

	ReleaseProc(session);

	mAuthPlayerNum--;
}

void procademy::CMMOServer::AuthTimeoutProc(CSession* session)
{
	ULONGLONG curTime = GetTickCount64();

	if (curTime - session->lastRecvTime > mTimeOut)
	{
		Disconnect(session);
	}
}

void procademy::CMMOServer::SendLoopProc()
{
	int count = 0;

	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::SESSION_STATUS::en_AUTH_RUN:
		case CSession::SESSION_STATUS::en_GAME_RUN:
		{
			SendPacketProc(session);
			break;
		}
		case CSession::SESSION_STATUS::en_AUTH_RELEASE_REQ:
		{
			if (session->isSending == false)
			{
				session->status = CSession::SESSION_STATUS::en_AUTH_RELEASE;
			}
			break;
		}
		case CSession::SESSION_STATUS::en_GAME_RELEASE_REQ:
		{
			if (session->isSending == false)
			{
				session->status = CSession::SESSION_STATUS::en_GAME_RELEASE;
			}
			break;
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
		//IncrementIOProc(session, 9999);
		SendPost(session);
		//DecrementIOProc(session, 9999);
	}
}

void procademy::CMMOServer::GameLoopProc()
{
	int transferCount = 0;

	for (int i = 0; i < mMaxClient; ++i)
	{
		CSession* session = mSessionArray[i];

		switch (session->status)
		{
		case CSession::SESSION_STATUS::en_GAME_READY:
		{
			if (transferCount < mMaxTransferToGame)
			{
				GameReadySessionProc(session);
				transferCount++;
			}
			break;
		}
		case CSession::SESSION_STATUS::en_GAME_RUN:
		{
			GameCompleteRecvProc(session);
			GameTimeoutProc(session);
			GameSessionToReleaseProc(session);
			break;
		}
		case CSession::SESSION_STATUS::en_GAME_RELEASE:
		{
			GameReleaseProc(session);
			break;
		}
		default:
			break;
		}
	}

	OnGame_Update();
}

void procademy::CMMOServer::GameReadySessionProc(CSession* session)
{
	session->OnGame_ClientJoin();

	mGamePlayerNum++;

	session->status = CSession::SESSION_STATUS::en_GAME_RUN;
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

		packet->SubRef();
	}

	if (count > 0)
	{
		session->lastRecvTime = GetTickCount64();
	}
}

void procademy::CMMOServer::GameSessionToReleaseProc(CSession* session)
{
	if (session->sessionEnd)
	{
		session->status = CSession::SESSION_STATUS::en_GAME_RELEASE_REQ;
	}
}

void procademy::CMMOServer::GameReleaseProc(CSession* session)
{
	session->OnGame_ClientLeave();
	session->OnGame_ClientRelease();
	
	ReleaseProc(session);

	mGamePlayerNum--;
}

void procademy::CMMOServer::GameTimeoutProc(CSession* session)
{
	ULONGLONG curTime = GetTickCount64();

	if (curTime - session->lastRecvTime > mTimeOut)
	{
		Disconnect(session);
	}
}


