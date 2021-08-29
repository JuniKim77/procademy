#include "CLanServer.h"
#include "CLogger.h"

//SOCKET CLanServer::mListenSocket = 0;

CLanServer::~CLanServer()
{
    closesocket(mListenSocket);
    if (mSessionPool != nullptr)
    {
        delete mSessionPool;
    }
    CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Lib End");
}

bool CLanServer::Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
    if (mbIsRunning == true) 
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Network is already running");
        return false;
    }

    mPort = port;
    mBindIP = ip;
    mWorkerThreadSize = createThread;
    mMaxRunThreadSize = runThread;
    mMaxClient = maxClient;
    mbNagle = nagle;
    mSessionPool = new procademy::ObjectPool<Session>(10);

    if (!CreateListenSocket())
    {
        return false;
    }

    BeginThreads();    

    mbIsRunning = true;

    return true;
}

bool CLanServer::Start(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
{
    return Start(port, INADDR_ANY, createThread, runThread, nagle, maxClient);
}

void CLanServer::Stop()
{
    CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Stop");

    closesocket(mListenSocket);

    mbIsRunning = false;
}

int CLanServer::GetSessionCount()
{
    return 0;
}

bool CLanServer::Disconnect(SESSION_ID SessionID)
{
    return false;
}

bool CLanServer::SendPacket(SESSION_ID SessionID, CPacket* packet)
{
    return false;
}

void CLanServer::LockSessionMap()
{
    AcquireSRWLockExclusive(&mSessionMapLock);
}

void CLanServer::UnlockSessionMap()
{
    ReleaseSRWLockExclusive(&mSessionMapLock);
}

void CLanServer::InsertSessionData(u_int64 sessionNo, Session* session)
{
    mSessionMap[sessionNo] = session;
}

void CLanServer::DeleteSessionData(u_int64 sessionNo)
{
    std::unordered_map<u_int64, Session*>::iterator iter = mSessionMap.find(sessionNo);

    if (iter == mSessionMap.end())
    {
        CLogger::_Log(dfLOG_LEVEL_DEBUG, L"WSAStartup", WSAGetLastError());
        return;
    }

    mSessionPool->Lock(false);
    mSessionPool->Free(iter->second);
    mSessionPool->Unlock(false);

    mSessionMap.erase(iter);
}

void CLanServer::UpdateSessionData(u_int64 sessionNo, Session* session)
{
    mSessionMap[sessionNo] = session;
}

bool CLanServer::CreateListenSocket()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup", WSAGetLastError());
        return false;
    }

    mListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mListenSocket == INVALID_SOCKET)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create socket", WSAGetLastError());
        return false;
    }

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mPort);
    addr.sin_addr.S_un.S_addr = htonl(mBindIP);

    int bindRet = bind(mListenSocket, (SOCKADDR*)&addr, sizeof(addr));

    if (bindRet == SOCKET_ERROR)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Bind", WSAGetLastError());
        closesocket(mListenSocket);
        return false;
    }

    // 백로그 길이?
    int listenRet = listen(mListenSocket, SOMAXCONN);

    if (listenRet == SOCKET_ERROR)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Listen", WSAGetLastError());
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
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort", WSAGetLastError());
        closesocket(mListenSocket);
        return false;
    }

    return true;
}

bool CLanServer::BeginThreads()
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

unsigned int __stdcall CLanServer::WorkerThread(LPVOID arg)
{
    CLanServer* server = (CLanServer*)arg;



    return 0;
}

unsigned int __stdcall CLanServer::AcceptThread(LPVOID arg)
{
    CLanServer* server = (CLanServer*)arg;

    while (1)
    {
        SOCKADDR_IN clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET client = accept(server->mListenSocket, (SOCKADDR*)&clientAddr, &addrLen);

        if (client == INVALID_SOCKET)
        {
            int err = WSAGetLastError();

            if (err == WSAENOTSOCK)
            {
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"ListenSocket Error", err);

                break;
            }

            CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Accept Error", err);

            continue;
        }

        if (server->mbZeroCopy)
        {
            int optNum = 0;
            if (setsockopt(client, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
            {
                CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy Error", WSAGetLastError());
                closesocket(client);
                
                continue;
            }
        }

        server->mSessionPool->Lock(false);
        Session* session = server->mSessionPool->Alloc();
        server->mSessionPool->Unlock(false);

        session->socket = client;
        session->ip = clientAddr.sin_addr.S_un.S_addr;
        session->port = clientAddr.sin_port;
        session->isSending = false;

        ZeroMemory(&session->send.overlapped, sizeof(WSAOVERLAPPED));
        ZeroMemory(&session->recv.overlapped, sizeof(WSAOVERLAPPED));
        InitializeSRWLock(&session->lock);

        HANDLE hResult = CreateIoCompletionPort((HANDLE)client, server->mHcp, (ULONG_PTR)session, 0);

        if (hResult == NULL)
        {
            CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort Error", WSAGetLastError());
            closesocket(server->mListenSocket);

            return -1;
        }

        server->LockSessionMap();
        server->InsertSessionData(server->mSessionIDCounter, session);
        server->UnlockSessionMap();

        session->ioCount = 1;
        server->RecvPost(session);
    }

    return 0;
}

bool CLanServer::RecvPost(Session* session)
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

        //wprintf_s(L"WSARecv ERROR, Error Code: %d\n", err);

        //DecrementProc(session);

        return false;
    }

    return true;
}

bool CLanServer::SendPost(Session* session)
{
    return false;
}

void CLanServer::SetWSABuf(WSABUF* bufs, Session* session, bool isRecv)
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

bool CLanServer::DecrementProc(Session* session)
{
    session->ioCount--;

    if (session->ioCount == 0)
    {
        /*WCHAR IP[16] = { 0, };

        InetNtop(AF_INET, &session->ip, IP, 16);
        wprintf_s(L"Disconnect [IP: %s] [Port: %u]\n", IP, ntohs(session->port));*/
        DisconnectProc(session);

        return true;
    }

    return false;
}

void CLanServer::DisconnectProc(Session* session)
{
    LockSessionMap();
    DeleteSessionData(session->sessionID);
    UnlockSessionMap();

    closesocket(session->socket);
}
