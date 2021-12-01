#include "CMMOServer.h"
#include "CLogger.h"
#include "TextParser.h"

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

	CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CNetServer Begin");

	if (!CreateListenSocket())
	{
		return false;
	}

	SetEvent(mBeginEvent);

	return true;
}

void procademy::CMMOServer::Stop()
{
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

	server->GameThreadProc();

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::AuthThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	server->AuthThreadProc();

	return 0;
}

unsigned int __stdcall procademy::CMMOServer::SendThread(LPVOID arg)
{
	CMMOServer* server = (CMMOServer*)arg;

	server->SendThreadProc();

	return 0;
}

void procademy::CMMOServer::Init()
{
	WORD		version = MAKEWORD(2, 2);
	WSADATA		data;

	WSAStartup(version, &data);
	CLogger::SetDirectory(L"_log");
	mBeginEvent = (HANDLE)CreateEvent(nullptr, false, false, nullptr);

	mhThreads = new HANDLE[(long long)mWorkerThreadNum + 2];
	mSessionArray = (CSession**)(new (CSession*)[mMaxClient]);

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
}

void procademy::CMMOServer::MonitorProc()
{
}

void procademy::CMMOServer::GQCSProc()
{
}

void procademy::CMMOServer::GameThreadProc()
{
}

void procademy::CMMOServer::AuthThreadProc()
{
}

void procademy::CMMOServer::SendThreadProc()
{
}
