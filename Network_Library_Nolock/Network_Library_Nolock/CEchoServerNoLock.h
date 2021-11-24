#include "CLanServerNoLock.h"
#include <unordered_set>
#include <unordered_map>

namespace procademy
{
	class CEchoServerNoLock : public CLanServerNoLock
	{
	public:
		CEchoServerNoLock();
		virtual bool	OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void	OnClientJoin(SESSION_ID SessionID) override;
		virtual void	OnClientLeave(SESSION_ID SessionID) override;
		virtual void	OnRecv(SESSION_ID SessionID, CNetPacket* packet) override;
		virtual void	OnError(int errorcode, const WCHAR* log) override;
		bool			BeginServer();
		void			WaitForThreadsFin();

	private:
		static unsigned int WINAPI	MonitorFunc(LPVOID arg);

		bool			MonitoringProc();
		void			InsertSessionID(u_int64 sessionNo);
		void			DeleteSessionID(u_int64 sessionNo);
		void			CompletePacket(SESSION_ID SessionID, CNetPacket* packet);
		void			EchoProc(SESSION_ID sessionID, CNetPacket* packet);
		void			LockMap();
		void			UnlockMap();
		void			MakeMonitorStr(WCHAR* s);
		void			BeginThreads();

	private:
		std::unordered_map<u_int64, int>	mSessionJoinMap;
		SRWLOCK								mSessionLock;
		HANDLE								mMonitoringThread = INVALID_HANDLE_VALUE;
	};
}
