#pragma once

#pragma comment(lib, "Pdh.lib")

#include <stdio.h>
#include <Pdh.h>

/// <summary>
/// CCpuUsage CPUTime();
/// </summary>

namespace procademy
{
	class CCpuUsage
	{
	private:
		struct st_ETHERNET
		{
			bool			bUse;
			WCHAR			szName[128];

			PDH_HCOUNTER	pdhCounterNetworkRecvBytes;
			PDH_HCOUNTER	pdhCounterNetworkSendBytes;
		};
	public:
		enum {
			PDH_ETHERNET_MAX = 8
		};

		CCpuUsage(HANDLE mhProcess = INVALID_HANDLE_VALUE);

		void		UpdateCpuTime();

		float		ProcessorTotal() { return mProcessorTotal; }
		float		ProcessorUser() { return mProcessorUser; }
		float		ProcessorKernel() { return mProcessorKernel; }

		float		ProcessTotal() { return mProcessTotal; }
		float		ProcessUser() { return mProcessUser; }
		float		ProcessKernel() { return mProcessKernel; }

		LONGLONG	ProcessUserMemory() { return mProcessUserMemory; }
		LONGLONG	ProcessNonPagedMemory() { return mProcessNonPagedMemory; }
		LONGLONG	AvailableMemory() { return mAvailableMemory; }
		LONGLONG	NonPagedMemory() { return mNonPagedMemory; }
		DWORD		ProcessHandleCount() { return mProcessHandleCount; }
		DWORD		ProcessThreadCount() { return mProcessThreadCount; }
		LONGLONG	NetworkRecvBytes() { return mPdhValueNetwork_RecvBytes; }
		LONGLONG	NetworkSendBytes() { return mPdhValueNetwork_SendBytes; }

	private:
		bool		GetProcessName(WCHAR* output);
		bool		GetEthernetCounters();

	private:
		HANDLE					mhProcess = INVALID_HANDLE_VALUE;
		int						mNumberOfProcessors = 0;

		float					mProcessorTotal = 0;
		float					mProcessorUser = 0;
		float					mProcessorKernel = 0;

		float					mProcessTotal = 0;
		float					mProcessUser = 0;
		float					mProcessKernel = 0;

		ULARGE_INTEGER			mProcessor_LastKernel;
		ULARGE_INTEGER			mProcessor_LastUser;
		ULARGE_INTEGER			mProcessor_LastIdle;

		ULARGE_INTEGER			mProcess_LastKernel;
		ULARGE_INTEGER			mProcess_LastUser;
		ULARGE_INTEGER			mProcess_LastTime;

		PDH_HQUERY				mCpuQuery;
		WCHAR					mProcessName[MAX_PATH]; // NAME

		PDH_HCOUNTER			mProcessUserMemoryCounter; // "\\Process(NAME)\\Private Bytes"
		LONGLONG				mProcessUserMemory;
		PDH_HCOUNTER			mProcessNonPagedMemoryCounter; // "\\Process(NAME)\\Pool Nonpaged Bytes"
		LONGLONG				mProcessNonPagedMemory;
		PDH_HCOUNTER			mAvailableMemoryCounter; // L"\\Memory\\Available MBytes"
		LONGLONG				mAvailableMemory;
		PDH_HCOUNTER			mNonPagedMemoryCounter; // L"\\Memory\\Pool Nonpaged Bytes"
		LONGLONG				mNonPagedMemory;
		PDH_HCOUNTER			mProcessHandleCountCounter; // "\\Process(NAME)\\Handle Count"
		DWORD					mProcessHandleCount;
		PDH_HCOUNTER			mProcessThreadCountCounter; // "\\Process(NAME)\\Thread Count"
		DWORD					mProcessThreadCount;

		// Network
		st_ETHERNET				mEthernetStruct[PDH_ETHERNET_MAX];
		LONGLONG				mPdhValueNetwork_RecvBytes; // Total
		LONGLONG				mPdhValueNetwork_SendBytes; // Total
	};
}
