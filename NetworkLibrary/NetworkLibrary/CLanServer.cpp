#include "CLanServer.h"
#include "CLogger.h"
#include "CPacket.h"

CLanServer::~CLanServer()
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

    mhThreads = new HANDLE[createThread + 1];

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

    CloseSessions();

    mbIsRunning = false;
}

int CLanServer::GetSessionCount()
{
    return mSessionMap.size();
}

void CLanServer::WaitForThreadsFin()
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
                    wprintf_s(L"Unset ZeroCopy Mode\n");
                }
                else
                {
                    mbZeroCopy = true;
                    wprintf_s(L"Set ZeroCopy Mode\n");
                }
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x4D) & 0x8001) // M
            {
                if (mbMonitoring)
                {
                    mbMonitoring = false;
                    wprintf_s(L"Unset monitoring Mode\n");
                }
                else
                {
                    mbMonitoring = true;
                    wprintf_s(L"Set monitoring Mode\n");
                }
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x50) & 0x8001) // P
            {
                for (auto iter = mSessionMap.begin(); iter != mSessionMap.end(); ++iter)
                {
                    int recvSize = iter->second->recv.queue.GetUseSize();
                    int sendSize = iter->second->send.queue.GetUseSize();

                    if (recvSize > 0 || sendSize > 0)
                    {
                        wprintf_s(L"[Socket: %u] [RecvUse: %d] [SendUse: %d] [Sending: %d] [io_Count: %d]\n",
                            iter->second->socket, recvSize, sendSize, iter->second->isSending, iter->second->ioCount);
                    }
                }
            }

            if (GetAsyncKeyState(VK_SHIFT) & 0x8001 && GetAsyncKeyState(0x49) & 0x8001) // I
            {
                for (auto iter = mSessionMap.begin(); iter != mSessionMap.end(); ++iter)
                {
                    int recvSize = iter->second->recv.queue.GetUseSize();
                    int sendSize = iter->second->send.queue.GetUseSize();

					wprintf_s(L"[Socket: %u] [RecvUse: %d] [SendUse: %d] [Sending: %d] [io_Count: %d]\n",
						iter->second->socket, recvSize, sendSize, iter->second->isSending, iter->second->ioCount);

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

bool CLanServer::Disconnect(SESSION_ID SessionID)
{
    Session* session = FindSession(SessionID);

    DisconnectProc(session);

    return true;
}

bool CLanServer::SendPacket(SESSION_ID SessionID, CPacket* packet)
{
    LockSessionMap();
    Session* session = FindSession(SessionID);

    LockSession(session);
    UnlockSessionMap();
    /*st_NETWORK_HEADER header;

    header.byCode = dfNETWORK_CODE;
    header.wPayloadSize = packet->GetSize();*/

    //session->send.queue.Lock(false);
    //session->send.queue.Enqueue((char*)&header, sizeof(header));
    session->send.queue.Enqueue(packet->GetFrontPtr(), packet->GetSize());
    //session->send.queue.Unlock(false);
    
    bool ret = SendPost(session);
    UnlockSession(session);

    if (ret == false)
    {
        DisconnectProc(session);
    }

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

        //CLogger::_Log(dfLOG_LEVEL_DEBUG, L"WSARecv ERROR: ", err);

        LockSession(session);
        DecrementProc(session);
        UnlockSession(session);

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

        return DecrementProc(session);
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
        return false;
    }

    return true;
}

void CLanServer::DisconnectProc(Session* session)
{
    u_int64 id = session->sessionID;

    closesocket(session->socket);

    LockSessionMap();
    DeleteSessionData(id);
    UnlockSessionMap();

    MonitorLock();
    mMonitor.disconnectCount++;
    MonitorUnlock();

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

        packet.MoveRear(ret);

        OnRecv(session->sessionID, &packet);
        //count += (sizeof(header) + header.wPayloadSize);
        count += ret;

        packet.Clear();
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

    //CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Socket Accept [IP: %s] [Port: %u]\n",
    //    IP, ntohs(clientAddr.sin_port));

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
    session->send.queue.ClearBuffer();
    session->send.queue.InitializeLock();
    session->recv.queue.ClearBuffer();
    session->recv.queue.InitializeLock();

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

    MonitorLock();
    mMonitor.acceptCount++;
    MonitorUnlock();

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
        LockSession(session);
        bool ret = DecrementProc(session);
        UnlockSession(session);

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
        LockSession(session);

        session->isSending = false;
        ZeroMemory(&session->send.overlapped, sizeof(session->send.overlapped));

        bool ret = DecrementProc(session);

        if (ret)
        {
            session->send.queue.MoveFront(transferredSize);

            if (session->send.queue.GetUseSize() > 0)
            {
                ret = SendPost(session);
            }
        }

        UnlockSession(session);

        if (ret == false)
        {
            DisconnectProc(session);
        }
    }

    return true;
}

void CLanServer::CloseSessions()
{
    for (auto iter = mSessionMap.begin(); iter != mSessionMap.end(); ++iter)
    {
        DisconnectProc(iter->second);
    }
}

void CLanServer::MonitorProc()
{
    if (mbMonitoring)
    {
        mSessionPool->Lock(true);
        int poolSize = mSessionPool->GetSize();
        int poolCapa = mSessionPool->GetCapacity();
        mSessionPool->Unlock(true);

        wprintf_s(L"=======================================\n[Total Accept Count: %u]\n[Total Diconnect Count: %u]\n[Live Session Count: %u]\n\
=======================================\n[Send TPS: %u]\n[Recv TPS: %u]\n[Pool Usage: (%d / %d)]\n=======================================\n",
        mMonitor.acceptCount,
        mMonitor.disconnectCount,
        mMonitor.acceptCount - mMonitor.disconnectCount,
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

void CLanServer::MonitorLock()
{
    AcquireSRWLockExclusive(&mMonitor.lock);
}

void CLanServer::MonitorUnlock()
{
    ReleaseSRWLockExclusive(&mMonitor.lock);
}
