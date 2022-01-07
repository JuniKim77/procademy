#include "CCpuUsage.h"
#include <stdio.h>

int main()
{
	procademy::CCpuUsage CPUTime;

	while (1)
	{
		WCHAR bigNumber[15];

		CPUTime.UpdateProcessorCpuTime();
		wprintf_s(L"\n\n===================================\n\n");
		wprintf_s(L"Processor:%f / Process:%f \n", CPUTime.ProcessorTotal(), CPUTime.ProcessTotal());
		wprintf_s(L"ProcessorKernel:%f / ProcessKernel:%f \n", CPUTime.ProcessorKernel(), CPUTime.ProcessKernel());
		wprintf_s(L"ProcessorUser:%f / ProcessUser: %f \n", CPUTime.ProcessorUser(), CPUTime.ProcessUser());
		CPUTime.GetBigNumberStr(CPUTime.ProcessUserMemory(), bigNumber, 15);
		wprintf_s(L"ProcessUserMemory:%s\n", bigNumber);
		CPUTime.GetBigNumberStr(CPUTime.ProcessNonPagedMemory(), bigNumber, 15);
		wprintf_s(L"ProcessNonPagedMemory:%s\n", bigNumber);
		CPUTime.GetBigNumberStr(CPUTime.AvailableMemory(), bigNumber, 15);
		wprintf_s(L"AvailableMemory:%s\n", bigNumber);
		CPUTime.GetBigNumberStr(CPUTime.NonPagedMemory(), bigNumber, 15);
		wprintf_s(L"NonPagedMemory:%s\n", bigNumber);
		wprintf_s(L"ProcessHandleCount:%d\n", CPUTime.ProcessHandleCount());
		wprintf_s(L"ProcessThreadCount:%d\n", CPUTime.ProcessThreadCount());
		CPUTime.GetBigNumberStr(CPUTime.NetworkRecvBytes(), bigNumber, 15);
		wprintf_s(L"NetworkRecvBytes:%s\n", bigNumber);
		CPUTime.GetBigNumberStr(CPUTime.NetworkSendBytes(), bigNumber, 15);
		wprintf_s(L"NetworkSendBytes:%s\n", bigNumber);
		
		Sleep(1000);
	}

	return 0;
}