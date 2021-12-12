#pragma once

#include "CPlayer.h"
#include "CMMOServer.h"
#include "CCpuUsage.h"

namespace procademy
{
	class CMMOEchoServer : public CMMOServer
	{
		friend class CPlayer;
	public:
		CMMOEchoServer();
		virtual ~CMMOEchoServer();
		bool			BeginServer();

	private:
		virtual void	AllocSessions(int num) override;
		virtual bool	OnConnectionRequest(u_long IP, u_short Port) override;
		virtual void	OnError(int errorcode, const WCHAR* log) override;
		virtual void	OnAuth_Update() override;
		virtual void	OnGame_Update() override;
		void			WaitForThreadsFin();
	
		static unsigned int WINAPI MonitorThread(LPVOID arg);

		void			BeginThreads();
		void			LoadInitFile(const WCHAR* fileName);
		void			Init();
		void			MonitoringProc();
		void			MakeMonitorStr(WCHAR* s, int size);

	private:
		CPlayer*				mPlayers;
		HANDLE					mMonitorThread;
		bool					mbMonitoring = true;
		CCpuUsage				mCpuUsage;
		alignas(64) DWORD		mLoginCount = 0;
	};
}