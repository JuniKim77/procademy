#pragma once

#include "CSession.h"

namespace procademy
{
	class CMMOServer
	{
	protected:
		CMMOServer();
		virtual ~CMMOServer();
		bool Start();
		void Stop();
		void LoadInitFile(const WCHAR* fileName);
		void QuitServer();
		void SetZeroCopy(bool on);
		void SetNagle(bool on);

	private:

		static unsigned int WINAPI MonitorThread(LPVOID arg);
		static unsigned int WINAPI AcceptThread(LPVOID arg);
		static unsigned int WINAPI IocpWorkerThread(LPVOID arg);
		static unsigned int WINAPI GameThread(LPVOID arg);
		static unsigned int WINAPI AuthThread(LPVOID arg);
		static unsigned int WINAPI SendThread(LPVOID arg);
	
		void Init();
		bool CreateIOCP();
		bool CreateListenSocket();
		bool BeginThreads();
		void InitializeEmptyIndex();
		void AcceptProc();
		void MonitorProc();
		void GQCSProc();
		void GameThreadProc();
		void AuthThreadProc();
		void SendThreadProc();

	private:
		enum {
			SEND_BUF_SIZE = 8192
		};

		SOCKET					mListenSocket = INVALID_SOCKET;
		BYTE					mNumThreads = 0;
		HANDLE					mIOCP = INVALID_HANDLE_VALUE;
		HANDLE*					mhThreads = nullptr;
		CSession**				mSessionArray = nullptr;
		TC_LFStack<u_short>		mEmptyIndexes;
		u_short					mMaxClient = 0;
		u_short					mPort = 0;
		WCHAR					mBindIP[32];
		HANDLE					mBeginEvent = INVALID_HANDLE_VALUE;

		/// <summary>
		/// 모니터링 변수들
		/// </summary>
		alignas(64) DWORD		sendTPS;
		alignas(64) DWORD		recvTPS;
		alignas(64) DWORD		disconnectCount;
		DWORD					acceptTotal;
		DWORD					acceptTPS;

	protected:
		bool					mbNagle = true;
		bool					mbZeroCopy = true;
		bool					mbExit = false;
		bool					mbBegin = false;
		BYTE					mActiveThreadNum = 0;
		BYTE					mWorkerThreadNum = 0;
	};
}