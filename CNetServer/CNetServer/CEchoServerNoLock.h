#include "CNetServerNoLock.h"
#include <unordered_set>
#include <unordered_map>

namespace procademy
{
	class CEchoServerNoLock : public CNetServerNoLock
	{
	public:
		CEchoServerNoLock();
		virtual bool OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void OnClientJoin(SESSION_ID SessionID) override;
		virtual void OnClientLeave(SESSION_ID SessionID) override;
		virtual void OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void OnError(int errorcode, const WCHAR* log) override;
		bool BeginServer(u_short port, u_long ip, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient); // 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
		bool BeginServer(u_short port, BYTE createThread, BYTE runThread, bool nagle, u_short maxClient);

	private:
		void KeyCheckProc();
		void InsertSessionID(u_int64 sessionNo);
		void DeleteSessionID(u_int64 sessionNo);
		void CompletePacket(SESSION_ID SessionID, CNetPacket* packet);
		void EchoProc(SESSION_ID sessionID, CNetPacket* packet);
		void LockMap();
		void UnlockMap();
		void MonitorProc();

		static unsigned int WINAPI MonitoringThread(LPVOID arg);

	private:
		std::unordered_map<u_int64, int>	mSessionJoinMap;
		SRWLOCK								mSessionLock;
		HANDLE								mMonitoringThread;
		bool								mbIsQuit = false;
	};
}
