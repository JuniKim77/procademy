#include "CLanClient.h"
#include "CLogger.h"
#include "CLanPacket.h"
#include "CCrashDump.h"
#include "CProfiler.h"
#include "TextParser.h"

procademy::CLanClient::CLanClient()
{
    LoadInitFile(L"ClientInit.cnf");
    Init();
    BeginThreads();
}

procademy::CLanClient::~CLanClient()
{
    if (mhThreads != nullptr)
    {
        delete[] mhThreads;
    }
}

bool procademy::CLanClient::Connect(const WCHAR* serverIP, USHORT serverPort)
{
    wcscpy_s(mServerIP, _countof(mServerIP), serverIP);
    mServerPort = serverPort;

    if (CreateSocket() == false)
    {
        return false;
    }

    return true;
}

bool procademy::CLanClient::Disconnect()
{
    BOOL ret;

    IncrementIOProc(40000);

    if (mClient.ioBlock.releaseCount.isReleased == 1)
    {
        DecrementIOProc(40020);

        return false;
    }

    ret = CancelIoEx((HANDLE)mClient.socket, nullptr);

    DecrementIOProc(40040);

    return ret;
}

bool procademy::CLanClient::SendPacket(CLanPacket* packet)
{
    IncrementIOProc(20000);

    if (mClient.ioBlock.releaseCount.isReleased == 1)
    {
        DecrementIOProc(20020);

        return false;
    }

    packet->AddRef();
    mClient.sendQ.Enqueue(packet);

#ifdef PROFILE
    CProfiler::Begin(L"SendPost");
    SendPost(session);
    CProfiler::End(L"SendPost");
#else
    SendPost();
#endif // PROFILE

    DecrementIOProc(20020);
    
    return true;
}

void procademy::CLanClient::SetZeroCopy(bool on)
{
    int optNum = on ? 0 : SEND_BUF_SIZE;

    if (setsockopt(mClient.socket, SOL_SOCKET, SO_SNDBUF, (char*)&optNum, sizeof(optNum)) == SOCKET_ERROR)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Zero Copy [Error: %d]", WSAGetLastError());
        closesocket(mClient.socket);

        return;
    }

    if (on)
    {
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Zero Copy On");
    }
    else
    {
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Zero Copy Off");
    }
}

void procademy::CLanClient::SetNagle(bool on)
{
    BOOL optval = on;

    if (setsockopt(mClient.socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socketopt Nagle [Error: %d]", WSAGetLastError());
        closesocket(mClient.socket);

        return;
    }

    if (on)
    {
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Nagle On");
    }
    else
    {
        CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Nagle Off");
    }
}

unsigned int __stdcall procademy::CLanClient::WorkerThread(LPVOID arg)
{
    CLanClient* client = (CLanClient*)arg;

    while (!client->mbExit)
    {
        client->GQCS();
    }

    return 0;
}

unsigned int __stdcall procademy::CLanClient::MonitorThread(LPVOID arg)
{
    CLanClient* client = (CLanClient*)arg;

    client->MonitorProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"CLanClient Monitor Thread End");

    return 0;
}

void procademy::CLanClient::Init()
{
    mhThreads = new HANDLE[(long long)mWorkerThreadNum + 1];

    CreateIOCP();
}

void procademy::CLanClient::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];

    tp.GetValue(L"IOCP_WORKER_THREAD", &num);
    mWorkerThreadNum = (BYTE)num;

    tp.GetValue(L"IOCP_ACTIVE_THREAD", &num);
    mActiveThreadNum = (BYTE)num;

    tp.GetValue(L"NAGLE", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        mbNagle = true;
    else
        mbNagle = false;
}

bool procademy::CLanClient::CreateIOCP()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    if (mActiveThreadNum > si.dwNumberOfProcessors)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Setting: Max Running thread is larger than the number of processors");
    }

    mIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, mActiveThreadNum);

    if (mIocp == NULL)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());

        return false;
    }

    return true;
}

bool procademy::CLanClient::BeginThreads()
{
    BYTE i = 0;

    mhThreads[i++] = (HANDLE)_beginthreadex(nullptr, 0, MonitorThread, this, 0, nullptr);

    for (; i <= mWorkerThreadNum; ++i)
    {
        mhThreads[i] = (HANDLE)_beginthreadex(nullptr, 0, WorkerThread, this, 0, nullptr);
    }

    mNumThreads = i;

    return true;
}

bool procademy::CLanClient::CreateSocket()
{
    WSADATA			wsa;
    SOCKADDR_IN		addr;
    int             connectRetval;
    HANDLE          hResult;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"WSAStartup [Error: %d]", WSAGetLastError());
        return false;
    }

    mClient.socket = socket(AF_INET, SOCK_STREAM, 0);
    if (mClient.socket == INVALID_SOCKET)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Create socket [Error: %d]", WSAGetLastError());
        return false;
    }
    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Create");

    addr.sin_family = AF_INET;
    addr.sin_port = htons(mServerPort);
    InetPton(AF_INET, mServerIP, &addr.sin_addr);

    connectRetval = connect(mClient.socket, (SOCKADDR*)&addr, sizeof(addr));

    if (connectRetval == SOCKET_ERROR)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"Socket Connect [Error: %d]", WSAGetLastError());
        closesocket(mClient.socket);
        return false;
    }
    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Client Socket Connect");

    hResult = CreateIoCompletionPort((HANDLE)mClient.socket, mIocp, 0, 0);

    if (hResult == NULL)
    {
        CLogger::_Log(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort [Error: %d]", WSAGetLastError());
        closesocket(mClient.socket);

        return false;
    }

    if (mbZeroCopy)
    {
        SetZeroCopy(mbZeroCopy);
    }

    if (mbNagle)
    {
        SetNagle(mbNagle);
    }

    return true;
}

void procademy::CLanClient::GQCS()
{
    while (1)
    {
        DWORD transferredSize = 0;
        Session* completionKey = nullptr;
        WSAOVERLAPPED* pOverlapped = nullptr;

        BOOL gqcsRet = GetQueuedCompletionStatus(mIocp, &transferredSize, (PULONG_PTR)&completionKey, &pOverlapped, INFINITE);
    
        // Client End
        if (transferredSize == 0 && (PULONG_PTR)completionKey == nullptr && pOverlapped == nullptr)
        {
            PostQueuedCompletionStatus(mIocp, 0, 0, 0);

            return;
        }

        if (pOverlapped == nullptr) // I/O Fail
        {
            OnError(10000, L"IOCP Error");

            return;
        }

        if (transferredSize != 0)
        {
            if (pOverlapped == &mClient.recvOverlapped) // Recv
            {
#ifdef PROFILE
                CProfiler::End(L"WSARecv");
                CProfiler::Begin(L"CompleteRecv");
                CompleteRecv(session, transferredSize);
                CProfiler::End(L"CompleteRecv");
#else
                CompleteRecv(transferredSize);
#endif // PROFILE
            }
            else // Send
            {
#ifdef PROFILE
                CProfiler::End(L"WSASend");
                CProfiler::Begin(L"CompleteSend");
                CompleteSend(session, transferredSize);
                CProfiler::End(L"CompleteSend");
#else
                CompleteSend(transferredSize);
#endif // PROFILE
            }

            SendPost();
        }

        DecrementIOProc(10000);
    }
}

void procademy::CLanClient::CompleteRecv(DWORD transferredSize)
{
    mClient.recvQ.MoveRear(transferredSize);
    USHORT len;
    DWORD count = 0;
    bool status = true;

    while (count < transferredSize)
    {
        if (mClient.recvQ.GetUseSize() <= sizeof(USHORT))
            break;

        mClient.recvQ.Peek((char*)&len, sizeof(USHORT));

        if (len > CLanPacket::eBUFFER_DEFAULT)
        {
            status = false;
            break;
        }

        if (mClient.recvQ.GetUseSize() < (sizeof(USHORT) + len))
            break;

        //CProfiler::Begin(L"ALLOC");
        CLanPacket* packet = CLanPacket::AllocAddRef();
        //CProfiler::End(L"ALLOC");

        memcpy_s(packet->GetZeroPtr(), sizeof(USHORT), (char*)&len, sizeof(USHORT));

        InterlockedIncrement(&recvTPS);

        mClient.recvQ.MoveFront(sizeof(USHORT));

        int ret = mClient.recvQ.Dequeue(packet->GetFrontPtr(), (int)len);

        packet->MoveRear(ret);
        OnRecv(packet); // -> SendPacket

        count += (ret + sizeof(SHORT));
        packet->SubRef();
    }
#ifdef PROFILE
    CProfiler::Begin(L"RecvPost");
    if (status)
    {
        RecvPost(session);
    }
    CProfiler::End(L"RecvPost");
#else
    if (status)
    {
        RecvPost();
    }
#endif // PROFILE
}

void procademy::CLanClient::CompleteSend(DWORD transferredSize)
{
    CLanPacket* packet;
    InterlockedAdd((LONG*)&sendTPS, mClient.numSendingPacket);
    for (int i = 0; i < mClient.numSendingPacket; ++i)
    {
        mClient.sendQ.Dequeue(&packet);

        packet->SubRef();
        packet = nullptr;
    }

    mClient.numSendingPacket = 0;
    mClient.isSending = false;
#ifdef PROFILE
    CProfiler::Begin(L"SendPost");
    SendPost(session);
    CProfiler::End(L"SendPost");
#else
    SendPost();
#endif // PROFILE
}

void procademy::CLanClient::MonitorProc()
{
    HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

    while (!mbExit)
    {
        DWORD retval = WaitForSingleObject(dummyEvent, 1000);

        if (retval == WAIT_TIMEOUT)
        {
            mMonitor.prevRecvTPS = recvTPS;
            mMonitor.prevSendTPS = sendTPS;

            recvTPS = 0;
            sendTPS = 0;
        }
    }

    CloseHandle(dummyEvent);
}

bool procademy::CLanClient::RecvPost(bool isFirst)
{
    if (!isFirst)
    {
        IncrementIOProc(30000);
    }

    WSABUF buffers[2];
    DWORD flags = 0;

    SetWSABuf(buffers, true);

    int recvRet = WSARecv(mClient.socket, buffers, 2, nullptr, &flags, &mClient.recvOverlapped, nullptr);

    if (recvRet == SOCKET_ERROR)
    {
        int err = WSAGetLastError();

        if (err == WSA_IO_PENDING)
        {
            return true;
        }

        DecrementIOProc(10050);

        return false;
    }

    return true;
}

bool procademy::CLanClient::SendPost()
{
    WSABUF buffers[100];
    bool ret = true;

    do
    {
        if (InterlockedExchange8((char*)&mClient.isSending, (char)true) == (char)true)
        {
            break;
        }

        if (mClient.sendQ.IsEmpty() == true)
        {
            mClient.isSending = false;

            if (mClient.sendQ.IsEmpty() == false)
            {
                continue;
            }

            break;
        }

        SetWSABuf(buffers, false);

        ZeroMemory(&mClient.sendOverlapped, sizeof(WSAOVERLAPPED));

        IncrementIOProc(30000);

#ifdef PROFILE
        CProfiler::Begin(L"WSASend");
#endif // PROFILE
        int sendRet = WSASend(mClient.socket, buffers, mClient.numSendingPacket, nullptr, 0, &mClient.sendOverlapped, nullptr);

        if (sendRet == SOCKET_ERROR)
        {
            int err = WSAGetLastError();

            if (err == WSA_IO_PENDING)
            {
                break;
            }

            DecrementIOProc(20050);

            ret = false;
        }

        break;
    } while (1);

    return ret;
}

void procademy::CLanClient::SetWSABuf(WSABUF* bufs, bool isRecv)
{
    if (isRecv)
    {
        char* pRear = mClient.recvQ.GetRearBufferPtr();
        char* pFront = mClient.recvQ.GetFrontBufferPtr();
        char* pBuf = mClient.recvQ.GetBuffer();
        int capa = mClient.recvQ.GetCapacity();

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
        CLanPacket* packetBufs[100];
        DWORD snapSize = mClient.sendQ.GetSize();

        if (snapSize > 100)
            snapSize = 100;

        if (mClient.sendQ.Peek(packetBufs, snapSize) != snapSize)
            CRASH();

        for (DWORD i = 0; i < snapSize; ++i)
        { 
            bufs[i].buf = packetBufs[i]->GetZeroPtr();
            bufs[i].len = packetBufs[i]->GetSize();
        }

        mClient.numSendingPacket = snapSize;
    }
}

void procademy::CLanClient::IncrementIOProc(int logic)
{
    InterlockedIncrement(&mClient.ioBlock.ioCount);
}

void procademy::CLanClient::DecrementIOProc(int logic)
{
    SessionIoCount ret;

    ret.ioCount = InterlockedDecrement(&mClient.ioBlock.ioCount);

    if (ret.releaseCount.count < 0)
    {
        CRASH();
    }

    if (ret.ioCount == 0)
    {
        ReleaseProc();
    }
}

void procademy::CLanClient::ReleaseProc()
{
    SessionIoCount released;
    CLanPacket* dummy;

    released.ioCount = 0;
    released.releaseCount.isReleased = 1;

    if (InterlockedCompareExchange(&mClient.ioBlock.ioCount, released.ioCount, 0) != 0)
    {
        return;
    }

    OnLeaveServer();

    closesocket(mClient.socket);

    mClient.isSending = false;

    while (1)
    {
        if (mClient.sendQ.Dequeue(&dummy) == false)
        {
            break;
        }

        /*USHORT ret = InterlockedIncrement16((SHORT*)&g_debugPacket2);
        g_sessionDebugs2[ret] = dummy;*/
        dummy->SubRef();
    }
    mClient.recvQ.ClearBuffer();

    ZeroMemory(&mClient.sendOverlapped, sizeof(WSAOVERLAPPED));
    ZeroMemory(&mClient.recvOverlapped, sizeof(WSAOVERLAPPED));
}
