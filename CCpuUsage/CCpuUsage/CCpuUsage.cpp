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
}
