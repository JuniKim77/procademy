#include "CEchoServerNoLock.h"
#include "CCrashDump.h"
#include "CNetPacket.h"
#include "MessageProtocol.h"
#include "CProfiler.h"
#include "CLogger.h"
#include <conio.h>

namespace procademy
{
	CEchoServerNoLock::CEchoServerNoLock()
	{
		LoadInitFile(L"ChatServer.cnf");
		BeginThreads();
		InitializeSRWLock(&mSessionLock);
	}

	bool CEchoServerNoLock::OnConnectionRequest(u_long IP, u_short Port)
	{
		return true;
	}

	void CEchoServerNoLock::OnClientJoin(SESSION_ID SessionID)
	{
		CProfiler::Begin(L"ALLOC");
		CNetPacket* packet = CNetPacket::AllocAddRef();
		CProfiler::End(L"ALLOC");

		WORD len = 8;
		int64_t value = 0x7fffffffffffffff;

		*packet << value;

		packet->SetHeader(true);

		SendPacket(SessionID, packet);
		InsertSessionID(SessionID);
		packet->SubRef();
	}

	void CEchoServerNoLock::OnClientLeave(SESSION_ID SessionID)
	{
		DeleteSessionID(SessionID);
	}

	void CEchoServerNoLock::OnRecv(SESSION_ID SessionID, CNetPacket* packet)
	{
		//CompletePacket(SessionID, packet);

		SendPacket(SessionID, packet);
	}

	void CEchoServerNoLock::OnError(int errorcode, const WCHAR* log)
	{
		
	}

	bool procademy::CEchoServerNoLock::BeginServer()
	{
		if (Start() == false)
		{
			CLogger::_Log(dfLOG_LEVEL_ERROR, L"Begin Server Error\n");

			return false;
		}

		WaitForThreadsFin();

		DWORD ret = WaitForSingleObject(mMonitoringThread, INFINITE);

		switch (ret)
		{
		case WAIT_FAILED:
			wprintf_s(L"EchoServer Thread Handle Error\n");
			break;
		case WAIT_TIMEOUT:
			wprintf_s(L"EchoServer Thread Timeout Error\n");
			break;
		case WAIT_OBJECT_0:
			wprintf_s(L"EchoServer None Error\n");
			break;
		default:
			break;
		}

		return true;
	}

	unsigned int __stdcall CEchoServerNoLock::MonitorFunc(LPVOID arg)
	{
		CEchoServerNoLock* echoServer = (CEchoServerNoLock*)arg;

		echoServer->MonitoringProc();

		wprintf(L"Monitoring Thread End\n");

		return 0;
	}

	bool CEchoServerNoLock::MonitoringProc()
	{
		HANDLE dummyevent = CreateEvent(nullptr, false, false, nullptr);
		WCHAR str[1024];

		while (!mbExit)
		{
			DWORD retval = WaitForSingleObject(dummyevent, 1000);

			if (retval == WAIT_TIMEOUT)
			{
				// Ãâ·Â
				MakeMonitorStr(str);

				wprintf(str);
			}
		}

		CloseHandle(dummyevent);

		return true;
	}

	void CEchoServerNoLock::InsertSessionID(u_int64 sessionNo)
	{
		LockMap();
		mSessionJoinMap[sessionNo]++;
		if (mSessionJoinMap[sessionNo] > 1)
		{
			CRASH();
		}
		UnlockMap();
	}

	void CEchoServerNoLock::DeleteSessionID(u_int64 sessionNo)
	{
		LockMap();
		mSessionJoinMap[sessionNo]--;
		if (mSessionJoinMap[sessionNo] == 0)
		{
			mSessionJoinMap.erase(sessionNo);
		}
		else
		{
			CRASH();
		}
		UnlockMap();
	}

	void CEchoServerNoLock::CompletePacket(SESSION_ID SessionID, CNetPacket* packet)
	{
		/*st_MESSAGE_HEADER* header = (st_MESSAGE_HEADER*)packet->GetBufferPtr();

		switch (header->wMessageType)
		{
		default:
			EchoProc(SessionID, packet);
			break;
		}*/

		/*CNetPacket msg;

		while (packet->GetSize() > 0)
		{
			st_MESSAGE_HEADER header;

			*packet >> header.wPayloadSize;

			msg.PutData((char*)&header.wPayloadSize, sizeof(header));

			msg.PutData(packet->GetFrontPtr(), header.wPayloadSize);

			packet->MoveFront(header.wPayloadSize);

			SendPacket(SessionID, &msg);

			msg.Clear();
		}*/
	}

	void CEchoServerNoLock::EchoProc(SESSION_ID sessionID, CNetPacket* packet)
	{
		SendPacket(sessionID, packet);
	}

	void CEchoServerNoLock::LockMap()
	{
		AcquireSRWLockExclusive(&mSessionLock);
	}

	void CEchoServerNoLock::UnlockMap()
	{
		ReleaseSRWLockExclusive(&mSessionLock);
	}

	void CEchoServerNoLock::MakeMonitorStr(WCHAR* s)
	{
		LONGLONG idx = 0;
		int len;

		idx += swprintf_s(s + idx, 1024 - idx, L"\n========================================\n");
		//idx += swprintf_s(s + idx, 1024 - idx, L"");
		idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%lld\n", L"Session Num : ", mSessionJoinMap.size());
		idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
		idx += swprintf_s(s + idx, 1024 - idx, L"%22sAlloc %d | Use %u\n", L"Packet Pool : ", CNetPacket::sPacketPool.GetCapacity(), CNetPacket::sPacketPool.GetSize());
		idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Accept Total : ", mMonitor.acceptTotal);
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Accept TPS : ", mMonitor.acceptTPS);
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Update TPS : ", mMonitor.acceptTPS);
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Recv TPS : ", mMonitor.prevRecvTPS);
		idx += swprintf_s(s + idx, 1024 - idx, L"%22s%u\n", L"Send TPS : ", mMonitor.prevSendTPS);
		idx += swprintf_s(s + idx, 1024 - idx, L"========================================\n");
	}
	void CEchoServerNoLock::BeginThreads()
	{
		mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitorFunc, this, 0, nullptr);
	}
}
