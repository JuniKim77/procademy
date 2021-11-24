#include "CChatServerMulti.h"
#include "CNetPacket.h"
#include "CommonProtocol.h"
#include "CLogger.h"
#include "TextParser.h"
#include <vector>
#include <conio.h>
#include "CProfiler.h"

#define MAX_STR (30000)

WCHAR str[MAX_STR];

procademy::CChatServerMulti::CChatServerMulti()
{
    Init();
	LoadInitFile(L"ChatServer.cnf");
	BeginThreads();
}

procademy::CChatServerMulti::~CChatServerMulti()
{
}

bool procademy::CChatServerMulti::OnConnectionRequest(u_long IP, u_short Port)
{
	return true;
}

void procademy::CChatServerMulti::OnClientJoin(SESSION_ID SessionID)
{
	JoinProc(SessionID);
}

void procademy::CChatServerMulti::OnClientLeave(SESSION_ID SessionID)
{
	LeaveProc(SessionID);
}

void procademy::CChatServerMulti::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
{

}

void procademy::CChatServerMulti::OnError(int errorcode, const WCHAR* log)
{
}

bool procademy::CChatServerMulti::BeginServer()
{
	HANDLE handles[2] = { mMonitoringThread, mHeartbeatThread };

	if (Start() == false)
	{
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error\n");

		return false;
	}

	WaitForThreadsFin();

	DWORD ret = WaitForMultipleObjects(2, handles, true, INFINITE);

	switch (ret)
	{
	case WAIT_FAILED:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Handle Error\n");
		break;
	case WAIT_TIMEOUT:
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"ChatServer Thread Timeout Error\n");
		break;
	case WAIT_OBJECT_0:
		CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer End\n");
		break;
	default:
		break;
	}

	return true;
}

void procademy::CChatServerMulti::WaitForThreadsFin()
{
    while (1)
    {
        char ch = _getch();

        switch (ch)
        {
        case 'g':
            mbNagle = !mbNagle;
            SetNagle(mbNagle);
            break;
        case 'z':
            mbZeroCopy = !mbZeroCopy;
            SetZeroCopy(mbZeroCopy);
            break;
        case 'm':
            if (mbMonitoring)
            {
                wprintf(L"Unset Monitoring\n");
                mbMonitoring = false;
            }
            else
            {
                wprintf(L"Set Monitoring\n");
                mbMonitoring = true;
            }
            break;
        case 's':
            if (mbBegin)
            {
                Stop();
                wprintf(L"STOP\n");
            }
            else
            {
                Start();
                wprintf(L"RUN\n");
            }
            break;
        case 'p':
            wprintf(L"Print Profiler\n");
            CProfiler::Print();
            break;
        case 'r':
            mbPrint = true;
            wprintf(L"Set Print Ratio\n");
            break;
        case 'd':
            CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"ChatServer Intended Crash\n");
            CRASH();
        case 'q':
            QuitServer();
            return;
        default:
            break;
        }
    }
}

void procademy::CChatServerMulti::Init()
{
    InitializeSRWLock(&mPlayerMapLock);
    InitializeSRWLock(&mSectorLock);
}

bool procademy::CChatServerMulti::CheckHeart()
{
	return false;
}

bool procademy::CChatServerMulti::MonitoringProc()
{
	return false;
}

bool procademy::CChatServerMulti::JoinProc(SESSION_ID sessionNo)
{
	return false;
}

bool procademy::CChatServerMulti::LoginProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	return false;
}

bool procademy::CChatServerMulti::LeaveProc(SESSION_ID sessionNo)
{
	return false;
}

bool procademy::CChatServerMulti::MoveSectorProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	return false;
}

bool procademy::CChatServerMulti::SendMessageProc(SESSION_ID sessionNo, CNetPacket* packet)
{
	return false;
}

bool procademy::CChatServerMulti::HeartUpdateProc(SESSION_ID sessionNo)
{
	return false;
}

bool procademy::CChatServerMulti::CheckTimeOutProc()
{
	return false;
}

void procademy::CChatServerMulti::BeginThreads()
{
}

void procademy::CChatServerMulti::LoadInitFile(const WCHAR* fileName)
{
    TextParser  tp;
    int         num;
    WCHAR       buffer[MAX_PARSER_LENGTH];
    BYTE        code;
    BYTE        key;

    tp.LoadFile(fileName);

    tp.GetValue(L"PACKET_CODE", &num);
    code = (BYTE)num;
    CNetPacket::SetCode(code);

    tp.GetValue(L"PACKET_KEY", &num);
    key = (BYTE)num;
    CNetPacket::SetPacketKey(key);

#ifdef TLS_MEMORY_POOL_VER
    tp.GetValue(L"POOL_SIZE_CHECK", buffer);
    if (wcscmp(L"TRUE", buffer) == 0)
        CNetPacket::sPacketPool.OnOffCounting();
#endif // TLS_MEMORY_POOL_VER

    tp.GetValue(L"TIMEOUT_DISCONNECT", &mTimeOut);

    tp.GetValue(L"IOCP_ACTIVE_THREAD", &num);
}

void procademy::CChatServerMulti::FreePlayer(st_Player* player)
{
    //AcquireSRWLockExclusive(&mPlayerMapLock);
    player->accountNo = 0;
    player->lastRecvTime = 0;
    player->curSectorX = -1;
    player->curSectorY = -1;

    player->bLogin = false;

    mPlayerPool.Free(player);
}

procademy::st_Player* procademy::CChatServerMulti::FindPlayer(SESSION_ID sessionNo)
{
    std::unordered_map<u_int64, st_Player*>::iterator iter = mPlayerMap.find(sessionNo);

    if (iter == mPlayerMap.end())
        return nullptr;

    return iter->second;
}

void procademy::CChatServerMulti::InsertPlayer(SESSION_ID sessionNo, st_Player* player)
{
    mPlayerMap[sessionNo] = player;
}

void procademy::CChatServerMulti::DeletePlayer(SESSION_ID sessionNo)
{
    mPlayerMap.erase(sessionNo);
}

void procademy::CChatServerMulti::Sector_AddPlayer(WORD x, WORD y, st_Player* player)
{
    mSector[y][x].list.push_back(player);
}

void procademy::CChatServerMulti::Sector_RemovePlayer(WORD x, WORD y, st_Player* player)
{
    for (auto iter = mSector[y][x].list.begin(); iter != mSector[y][x].list.end();)
    {
        if ((*iter)->sessionNo == player->sessionNo)
        {
            mSector[y][x].list.erase(iter);

            return;
        }
        else
        {
            iter++;
        }
    }

    CLogger::_Log(dfLOG_LEVEL_ERROR, L"DeleteFromSector[X %u][Y %u] - Not Found Player[%lld]",
        x, y, player->accountNo);

    CRASH();
}

void procademy::CChatServerMulti::GetSectorAround(WORD x, WORD y, st_Sector_Around* output)
{
    WORD beginX = x == 0 ? 0 : x - 1;
    WORD beginY = y == 0 ? 0 : y - 1;
    WORD endX = x == 49 ? 49 : x + 1;
    WORD endY = y == 49 ? 49 : y + 1;

    output->count = 0;

    for (WORD i = beginY; i <= endY; ++i)
    {
        for (WORD j = beginX; j <= endX; ++j)
        {
            output->around[output->count].y = i;
            output->around[output->count].x = j;
            output->count++;
        }
    }
}

DWORD procademy::CChatServerMulti::SendMessageSectorAround(CNetPacket* packet, st_Sector_Around* input)
{
    DWORD ret = 0;

    for (int i = 0; i < input->count; ++i)
    {
        int curX = input->around[i].x;
        int curY = input->around[i].y;

        mSector[curY][curX].updateCount++;
        mSector[curY][curX].playerCount += mSector[curY][curX].list.size();
        ret += mSector[curY][curX].list.size();

        for (std::list<st_Player*>::iterator iter = mSector[curY][curX].list.begin(); iter != mSector[curY][curX].list.end(); ++iter)
        {
#ifdef SEND_TO_WORKER
            SendPacketToWorker((*iter)->sessionNo, packet);
#else
            SendPacket((*iter)->sessionNo, packet);
#endif // SEND_TO_WORKER
        }
    }

    return ret;
}

void procademy::CChatServerMulti::MakeMonitorStr(WCHAR* s, int size)
{
    LONGLONG idx = 0;
    int len;
    WCHAR bigNumber[18];

    idx += swprintf_s(s + idx, size - idx, L"\n========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"[Zero Copy: %d] [Nagle: %d]\n", mbZeroCopy, mbNagle);
    idx += swprintf_s(s + idx, size - idx, L"[WorkerTh: %d] [ActiveTh: %d]\n", mWorkerThreadNum, mActiveThreadNum);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%lld\n", L"Session Num : ", mPlayerMap.size());
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Player Num : ", mLoginCount);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
#ifdef TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %u\n", L"Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
#endif // TLS_MEMORY_POOL_VER
    idx += swprintf_s(s + idx, size - idx, L"%22sAlloc %d | Use %d\n", L"Player Pool : ", mPlayerPool.GetCapacity(), mPlayerPool.GetSize());
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Update TPS : ", mUpdateTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
    idx += swprintf_s(s + idx, size - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
    idx += swprintf_s(s + idx, size - idx, L"CPU usage [T:%.1f U:%.1f K:%.1f] [Chat:%.1f U:%.1f K%.1f]\n",
        mCpuUsage.ProcessorTotal(), mCpuUsage.ProcessorUser(), mCpuUsage.ProcessorKernel(),
        mCpuUsage.ProcessTotal(), mCpuUsage.ProcessUser(), mCpuUsage.ProcessKernel());
    mCpuUsage.GetBigNumberStr(mCpuUsage.ProcessUserMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"ProcessUserMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.ProcessNonPagedMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"ProcessNonPagedMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.AvailableMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"AvailableMemory : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.NonPagedMemory(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NonPagedMemory : ", bigNumber);
    idx += swprintf_s(s + idx, size - idx, L"%25s%d\n", L"ProcessHandleCount : ", mCpuUsage.ProcessHandleCount());
    idx += swprintf_s(s + idx, size - idx, L"%25s%d\n", L"ProcessThreadCount : ", mCpuUsage.ProcessHandleCount());
    mCpuUsage.GetBigNumberStr(mCpuUsage.NetworkRecvBytes(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NetworkRecvBytes : ", bigNumber);
    mCpuUsage.GetBigNumberStr(mCpuUsage.NetworkSendBytes(), bigNumber, 18);
    idx += swprintf_s(s + idx, size - idx, L"%25s%s\n", L"NetworkSendBytes : ", bigNumber);
    idx += swprintf_s(s + idx, size - idx, L"========================================\n");
}

void procademy::CChatServerMulti::PrintRecvSendRatio()
{
    mbPrint = false;

    FILE* fout = nullptr;
    int idx = 0;
    st_Sector_Around sectorAround;

    _wfopen_s(&fout, L"sectorRatio.csv", L"w");

    if (fout == nullptr)
        return;

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            if (mSector[i][j].recvCount != 0)
            {
                GetSectorAround(j, i, &sectorAround);
                double avgPlayers = 0.0;

                for (int i = 0; i < sectorAround.count; ++i)
                {
                    int curX = sectorAround.around[i].x;
                    int curY = sectorAround.around[i].y;

                    avgPlayers += mSector[curY][curX].playerCount / (double)mSector[curY][curX].updateCount;
                }

                idx += swprintf_s(str + idx, MAX_STR - idx, L"%.2lf<%.2lf>,",
                    mSector[i][j].sendCount / (double)mSector[i][j].recvCount,
                    avgPlayers);
            }
            else
            {
                idx += swprintf_s(str + idx, MAX_STR - idx, L"%6d,", 0);
            }
        }

        idx += swprintf_s(str + idx, MAX_STR - idx, L"\n");
    }

    fwprintf_s(fout, str);

    fclose(fout);
}

void procademy::CChatServerMulti::ClearTPS()
{
    mUpdateTPS = 0;

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            mSector[i][j].recvCount = 0;
            mSector[i][j].sendCount = 0;
            mSector[i][j].updateCount = 0;
            mSector[i][j].playerCount = 0;
        }
    }
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResLogin(BYTE status, SESSION_ID accountNo)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_LOGIN << status << (__int64)accountNo;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResSectorMove(SESSION_ID accountNo, WORD sectorX, WORD sectorY)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_SECTOR_MOVE << (__int64)accountNo << sectorX << sectorY;

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

procademy::CNetPacket* procademy::CChatServerMulti::MakeCSResMessage(SESSION_ID accountNo, WCHAR* ID, WCHAR* nickname, WORD meesageLen, WCHAR* message)
{
    CNetPacket* packet = CNetPacket::AllocAddRef();

    *packet << (WORD)en_PACKET_CS_CHAT_RES_MESSAGE << (__int64)accountNo;

    packet->PutData(ID, 20);
    packet->PutData(nickname, 20);
    *packet << meesageLen;
    packet->PutData(message, meesageLen / 2);

    packet->SetHeader(false);
    packet->Encode();

    return packet;
}

unsigned int __stdcall procademy::CChatServerMulti::MonitorFunc(LPVOID arg)
{
    CChatServerMulti* chatServer = (CChatServerMulti*)arg;

    chatServer->MonitoringProc();

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"Monitoring Thread End\n");

    return 0;
}

unsigned int __stdcall procademy::CChatServerMulti::HeartbeatFunc(LPVOID arg)
{
    CChatServerMulti* chatServer = (CChatServerMulti*)arg;

    while (!chatServer->mbExit)
    {
        chatServer->CheckHeart();
    }

    CLogger::_Log(dfLOG_LEVEL_SYSTEM, L"HeartBeat Thread End\n");

    return 0;
}
