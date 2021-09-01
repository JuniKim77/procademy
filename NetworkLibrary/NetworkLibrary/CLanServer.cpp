#include "CLanServer.h"
#include "CLogger.h"
#include "CPacket.h"
#include "Protocol.h"

CLanServer::~CLanServer()
{
    closesocket(mListenSocket);
    if (mSessionPool != nullptr)
    {
        delete mSessionPool;
    }
    CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Lib End\n");
}

bool CLanServer::Start(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
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
    if (mbIsRunning == false)
    {
        return;
    }

    CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Network Stop\n");

    closesocket(mListenSocket);

    mbIsRunning = false;
}

int CLanServer::GetSessionCount()
{
    return mSessionMap.size();
}

bool CLanServer::Disconnect(SESSION_ID SessionID)
{
    Session* session = FindSession(SessionID);

    DisconnectProc(session);

    return true;
}

bool CLanServer::SendPacket(SESSION_ID SessionID, CPacket* packet)
{
    Session* session = FindSession(SessionID);
    st_NETWORK_HEADER header;

    header.byCode = dfNETWORK_CODE;
    header.wPayloadSize = packet->GetSize();

    session->send.queue.Lock(false);
    session->send.queue.Enqueue((char*)&header, sizeof(header));
    session->send.queue.Enqueue(packet->GetBufferPtr(), packet->GetSize());
    session->send.queue.Unlock(false);

    LockSession(session);
    bool ret = SendPost(session);
    UnlockSession(session);

    return ret;
}

void CLanServer::LockSessionMap()
{
    AcquireSRWLockExclusive(&mSessionMapLock);
}

void CLanServer::UnlockSessionMap()
{
    ReleaseSRWLockExclusive(&mSessionMapLock);
}

CLanServer::Session* CLanServer::FindSession(u_int64 sessionNo)
{
    return mSessionMap[sessionNo];
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
        CLogger::_Log(dfLOG_LEVEL_DEBUG, L"WSAStartup [Error: %d]\n", WSAGetLastError());
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

    while (1)
    {
        if (server->OnCompleteMessage() == false)
        {
            break;
        }
    }

    return 0;
}

unsigned int __stdcall CLanServer::AcceptThread(LPVOID arg)
{
    CLanServer* server = (CLanServer*)arg;

    while (1)
    {
        if (server->AcceptProc() == false)
        {
            break;
        }
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

        // CLogger::_Log(dfLOG_LEVEL_DEBUG, L"WSARecv ERROR: ", err);

        DecrementProc(session);

        return false;
    }

    return true;
}

bool CLanServer::SendPost(Session* session)
{
    WSABUF buffers[2];

    if (session->isSending)
    {
        return true;
    }

    session->isSending = true;

    SetWSABuf(buffers, session, false);

    session->ioCount++;
    int sendRet = WSASend(session->socket, buffers, 2, nullptr, 0, &session->send.overlapped, nullptr);

    if (sendRet == SOCKET_ERROR)
    {
        int err = WSAGetLastError();

        if (err == WSA_IO_PENDING)
        {
            return true;
        }

        if (DecrementProc(session))
            return false;
    }

    return true;
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

        return false;
    }

    return true;
}

void CLanServer::DisconnectProc(Session* session)
{
    u_int64 id = session->sessionID;

    LockSessionMap();
    DeleteSessionData(id);
    UnlockSessionMap();

    closesocket(session->socket);

    OnClientLeave(id);
}

void CLanServer::LockSession(Session* session)
{
    AcquireSRWLockExclusive(&session->lock);
}

void CLanServer::UnlockSession(Session* session)
{
    ReleaseSRWLockExclusive(&session->lock);

}

void CLanServer::PacketProc(Session* session, DWORD msgSize)
{
    session->recv.queue.MoveRear(msgSize);

    int count = 0;

    while (count < msgSize)
    {
        CPacket packet;

        st_NETWORK_HEADER header;

        session->recv.queue.Dequeue((char*)&header, sizeof(header));

        session->recv.queue.Dequeue((char*)&packet, header.wPayloadSize);

        OnRecv(session->sessionID, &packet);
        count += (sizeof(header) + header.wPayloadSize);
    }
}

bool CLanServer::AcceptProc()
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

    session->ioCount = 1;
    RecvPost(session);
    OnClientJoin(session->sessionID);

    return true;
}

CLanServer::Session* CLanServer::CreateSession(SOCKET client, SOCKADDR_IN clientAddr)
{
    mSessionPool->Lock(false);
    Session* session = mSessionPool->Alloc();
    mSessionPool->Unlock(false);

    session->socket = client;
    session->ip = clientAddr.sin_addr.S_un.S_addr;
    session->port = clientAddr.sin_port;
    session->isSending = false;
    session->sessionID = mSessionIDCounter;

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

    LockSessionMap();
    InsertSessionData(mSessionIDCounter, session);
    mSessionIDCounter++;
    UnlockSessionMap();

    return session;
}

bool CLanServer::OnCompleteMessage()
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
        DecrementProc(session);

        return true;
    }

    if (pOverlapped == &session->recv.overlapped) // Recv
    {
        PacketProc(session, transferredSize);

        RecvPost(session);
    }
    else // Send
    {
        LockSession(session);

        session->send.queue.Lock(false);
        session->isSending = false;
        ZeroMemory(&session->send.overlapped, sizeof(session->send.overlapped));

        if (DecrementProc(session))
        {
            session->send.queue.MoveFront(transferredSize);

            if (session->send.queue.GetUseSize() > 0)
            {
                SendPost(session);
            }
        }
        session->send.queue.Unlock(false);

        UnlockSession(session);
    }

    return true;
}
