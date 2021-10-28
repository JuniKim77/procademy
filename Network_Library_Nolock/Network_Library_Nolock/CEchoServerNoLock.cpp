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

	bool CEchoServerNoLock::BeginServer(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
	{
		if (Start(port, ip, createThread, runThread, nagle, maxClient) == false)
		{
			return false;
		}

		mMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, MonitoringThread, this, 0, nullptr);

		KeyCheckProc();

		WaitForThreadsFin();

		return true;
	}

	bool CEchoServerNoLock::BeginServer(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient)
	{
		return BeginServer(port, INADDR_ANY, createThread, runThread, nagle, maxClient);
	}

	void CEchoServerNoLock::KeyCheckProc()
	{
		while (1)
		{
			char ch = _getch();

			switch (ch)
			{
			case 'Z':
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
				break;
			case 'G':
				if (mbNagle)
				{
					mbNagle = false;
					wprintf_s(L"\nUnset Nagle Mode\n\n");
				}
				else
				{
					mbNagle = true;
					wprintf_s(L"\nSet Nagle Mode\n\n");
				}
				break;
			case 'M':
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
				break;
			case 'P':
				CProfiler::Print();
				break;
			case 'D':
				CRASH();
				break;
			case 'Q':
				QuitServer();
				return;
			default:
				break;
			}
		}
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
	void CEchoServerNoLock::MonitorProc()
	{
		HANDLE dummyEvent = CreateEvent(nullptr, false, false, nullptr);

		while (!mbIsQuit)
		{
			DWORD retval = WaitForSingleObject(dummyEvent, 1000);

			if (retval == WAIT_TIMEOUT)
			{
				wprintf_s(L"\nMonitoring[M]: (%d) | Quit[Q]\n", mbMonitoring);
				wprintf_s(L"ZeroCopy[Z]: (%d) | Nagle[N]: (%d)\n", mbZeroCopy, mbNagle);
				wprintf_s(L"=======================================\n[Total Accept Count: %lu]\n[Total Diconnect Count: %lu]\n[Live Session Count: %lu]\n",
					mMonitor.acceptCount,
					mMonitor.disconnectCount,
					mMonitor.acceptCount - mMonitor.disconnectCount);
#ifdef NEW_DELETE_VER
				wprintf_s(L"[Packet Pool Capa: %d]\n[Packet Pool Use: %d]\n[Send TPS: %u]\n[Recv TPS: %u]\n=======================================\n",
					0,
					0,
					mMonitor.sendTPS,
					mMonitor.recvTPS);
#else
				wprintf_s(L"[Packet Pool Capa: %d]\n[Packet Pool Use: %d]\n[Send TPS: %u]\n[Recv TPS: %u]\n=======================================\n",
					CNetPacket::GetPoolCapacity(),
					CNetPacket::GetPoolSize(),
					mMonitor.prevSendTPS,
					mMonitor.prevRecvTPS);
#endif // NEW_DELETE_VER
			}
		}

		CloseHandle(dummyEvent);
	}
	unsigned int __stdcall CEchoServerNoLock::MonitoringThread(LPVOID arg)
	{
		CEchoServerNoLock* server = (CEchoServerNoLock*)arg;

		server->MonitorProc();

		return 0;
	}
}
