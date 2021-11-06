#include <Windows.h>
#include "CCpuUsage.h"

procademy::CCpuUsage::CCpuUsage(HANDLE mhProcess)
{
	if (mhProcess == INVALID_HANDLE_VALUE)
	{
		mhProcess = GetCurrentProcess();
	}

	SYSTEM_INFO SystemInfo;

	GetSystemInfo(&SystemInfo);
	mNumberOfProcessors = SystemInfo.dwNumberOfProcessors;

	mProcessor_LastKernel.QuadPart = 0;
	mProcessor_LastUser.QuadPart = 0;
	mProcessor_LastIdle.QuadPart = 0;

	mProcess_LastKernel.QuadPart = 0;
	mProcess_LastUser.QuadPart = 0;
	mProcess_LastTime.QuadPart = 0;

	PdhOpenQuery(NULL, NULL, &mCpuQuery);
	GetProcessName(mProcessName);

	WCHAR szQuery[1024];

	swprintf_s(szQuery, MAX_PATH, L"\\Process(%s)\\Private Bytes", mProcessName);
	PdhAddCounter(mCpuQuery, szQuery, NULL, &mProcessUserMemoryCounter);
	swprintf_s(szQuery, MAX_PATH, L"\\Process(%s)\\Pool Nonpaged Bytes", mProcessName);
	PdhAddCounter(mCpuQuery, szQuery, NULL, &mProcessNonPagedMemoryCounter);
	swprintf_s(szQuery, MAX_PATH, L"\\Process(%s)\\Handle Count", mProcessName);
	PdhAddCounter(mCpuQuery, szQuery, NULL, &mProcessHandleCountCounter);
	swprintf_s(szQuery, MAX_PATH, L"\\Process(%s)\\Thread Count", mProcessName);
	PdhAddCounter(mCpuQuery, szQuery, NULL, &mProcessThreadCountCounter);
	
	PdhAddCounter(mCpuQuery, L"\\Memory\\Available MBytes", NULL, &mAvailableMemoryCounter);
	PdhAddCounter(mCpuQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &mNonPagedMemoryCounter);

	GetEthernetCounters();

	UpdateCpuTime();
}

void procademy::CCpuUsage::UpdateCpuTime()
{
	ULARGE_INTEGER Idle;
	ULARGE_INTEGER Kernel;
	ULARGE_INTEGER User;

	if (GetSystemTimes((PFILETIME)&Idle, (PFILETIME)&Kernel, (PFILETIME)&User) == false)
	{
		return;
	}

	ULONGLONG KernelDiff = Kernel.QuadPart - mProcessor_LastKernel.QuadPart;
	ULONGLONG UserDiff = User.QuadPart - mProcessor_LastUser.QuadPart;
	ULONGLONG IdleDiff = Idle.QuadPart - mProcessor_LastIdle.QuadPart;

	ULONGLONG Total = KernelDiff + UserDiff;
	ULONGLONG TimeDiff;

	if (Total == 0)
	{
		mProcessorUser = 0.0f;
		mProcessorKernel = 0.0f;
		mProcessorTotal = 0.0f;
	}
	else
	{
		mProcessorTotal = (float)((double)(Total - IdleDiff) / Total * 100.0f);
		mProcessorUser = (float)((double)UserDiff / Total * 100.0f);
		mProcessorKernel = (float)((double)(KernelDiff - IdleDiff) / Total * 100.0f);
	}

	mProcessor_LastKernel = Kernel;
	mProcessor_LastUser = User;
	mProcessor_LastIdle = Idle;

	ULARGE_INTEGER None;
	ULARGE_INTEGER NowTime;

	GetSystemTimeAsFileTime((LPFILETIME)&NowTime);

	GetProcessTimes(mhProcess, (LPFILETIME)&None, (LPFILETIME)&None, (LPFILETIME)&Kernel, (LPFILETIME)&User);

	TimeDiff = NowTime.QuadPart - mProcess_LastTime.QuadPart;
	UserDiff = User.QuadPart - mProcess_LastUser.QuadPart;
	KernelDiff = Kernel.QuadPart - mProcess_LastKernel.QuadPart;

	Total = KernelDiff + UserDiff;

	mProcessTotal = (float)(Total / (double)mNumberOfProcessors / (double)TimeDiff * 100.0f);
	mProcessKernel = (float)(KernelDiff / (double)mNumberOfProcessors / (double)TimeDiff * 100.0f);
	mProcessUser = (float)(UserDiff / (double)mNumberOfProcessors / (double)TimeDiff * 100.0f);

	mProcess_LastTime = NowTime;
	mProcess_LastKernel = Kernel;
	mProcess_LastUser = User;

	PdhCollectQueryData(mCpuQuery);

	PDH_FMT_COUNTERVALUE CounterValue;

	PdhGetFormattedCounterValue(mProcessUserMemoryCounter, PDH_FMT_LARGE, NULL, &CounterValue);
	mProcessUserMemory = CounterValue.largeValue;
	PdhGetFormattedCounterValue(mProcessNonPagedMemoryCounter, PDH_FMT_LARGE, NULL, &CounterValue);
	mProcessNonPagedMemory = CounterValue.largeValue;
	PdhGetFormattedCounterValue(mAvailableMemoryCounter, PDH_FMT_LARGE, NULL, &CounterValue);
	mAvailableMemory = CounterValue.largeValue;
	PdhGetFormattedCounterValue(mNonPagedMemoryCounter, PDH_FMT_LARGE, NULL, &CounterValue);
	mNonPagedMemory = CounterValue.largeValue;
	PdhGetFormattedCounterValue(mProcessHandleCountCounter, PDH_FMT_LONG, NULL, &CounterValue);
	mProcessHandleCount = (DWORD)CounterValue.longValue;
	PdhGetFormattedCounterValue(mProcessThreadCountCounter, PDH_FMT_LONG, NULL, &CounterValue);
	mProcessThreadCount = (DWORD)CounterValue.longValue;

	mPdhValueNetwork_RecvBytes = 0;
	mPdhValueNetwork_SendBytes = 0;

	// Network
	for (int i = 0; i < PDH_ETHERNET_MAX; ++i)
	{
		if (mEthernetStruct[i].bUse)
		{
			PdhGetFormattedCounterValue(mEthernetStruct[i].pdhCounterNetworkRecvBytes,
				PDH_FMT_LONG, NULL, &CounterValue);

			mPdhValueNetwork_RecvBytes += (DWORD)CounterValue.longValue;

			PdhGetFormattedCounterValue(mEthernetStruct[i].pdhCounterNetworkSendBytes,
				PDH_FMT_LONG, NULL, &CounterValue);

			mPdhValueNetwork_SendBytes += (DWORD)CounterValue.longValue;
		}
	}
}

bool procademy::CCpuUsage::GetProcessName(WCHAR* output)
{
	bool ret = true;
	DWORD process_id = GetCurrentProcessId();
	HANDLE process_handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		process_id
	);

	WCHAR buffer[MAX_PATH] = {};
	DWORD buffer_size = MAX_PATH;
	DWORD i;

	if (process_handle) {
		if (!QueryFullProcessImageNameW(process_handle, 0, buffer, &buffer_size)) {
			ret = false;
		}
		else
		{
			for (i = buffer_size; i > 0; --i)
			{
				if (buffer[i] == L'.')
				{
					buffer[i] = L'\0';
				}

				if (buffer[i] == L'\\')
				{
					break;
				}
			}

			wcscpy_s(output, MAX_PATH, buffer + i + 1);
		}

		CloseHandle(process_handle);
	}

	return true;
}

bool procademy::CCpuUsage::GetEthernetCounters()
{
	DWORD dwCounterSize = 0;
	DWORD dwInterfaceSize = 0;
	WCHAR* szCounters = nullptr;
	WCHAR* szInterfaces = nullptr;

	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		delete[] szCounters;
		delete[] szInterfaces;

		return false;
	}

	WCHAR* szCur = szInterfaces;
	WCHAR szQuery[1024];
	int count = 0;

	for (; *szCur != L'\0' && count < PDH_ETHERNET_MAX; szCur += wcslen(szCur) + 1, count++)
	{
		mEthernetStruct[count].bUse = true;
		mEthernetStruct[count].szName[0] = L'\0';

		wcscpy_s(mEthernetStruct[count].szName, szCur);

		szQuery[0] = L'\0';
		swprintf_s(szQuery, _countof(szQuery), L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
		PdhAddCounter(mCpuQuery, szQuery, NULL, &mEthernetStruct[count].pdhCounterNetworkRecvBytes);

		szQuery[0] = L'\0';
		swprintf_s(szQuery, _countof(szQuery), L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
		PdhAddCounter(mCpuQuery, szQuery, NULL, &mEthernetStruct[count].pdhCounterNetworkSendBytes);
	}

	delete[] szCounters;
	delete[] szInterfaces;

	return true;
}
