#pragma once

#include <wtypes.h>

/// <summary>
/// CCpuUsage CPUTime();
/// </summary>

namespace procademy
{
	class CCpuUsage
	{
	public:
		CCpuUsage(HANDLE mhProcess = INVALID_HANDLE_VALUE);

		void	UpdateCpuTime();

		float	ProcessorTotal() { return mProcessorTotal; }
		float	ProcessorUser() { return mProcessorUser; }
		float	ProcessorKernel() { return mProcessorKernel; }

		float	ProcessTotal() { return mProcessTotal; }
		float	ProcessUser() { return mProcessUser; }
		float	ProcessKernel() { return mProcessKernel; }

	private:
		HANDLE				mhProcess = INVALID_HANDLE_VALUE;
		int					mNumberOfProcessors = 0;

		float				mProcessorTotal = 0;
		float				mProcessorUser = 0;
		float				mProcessorKernel = 0;

		float				mProcessTotal = 0;
		float				mProcessUser = 0;
		float				mProcessKernel = 0;

		ULARGE_INTEGER		mProcessor_LastKernel;
		ULARGE_INTEGER		mProcessor_LastUser;
		ULARGE_INTEGER		mProcessor_LastIdle;

		ULARGE_INTEGER		mProcess_LastKernel;
		ULARGE_INTEGER		mProcess_LastUser;
		ULARGE_INTEGER		mProcess_LastTime;
	};
}