#include "CCpuUsage.h"
#include <stdio.h>

int main()
{
	procademy::CCpuUsage CPUTime;

	while (1)
	{
		CPUTime.UpdateCpuTime();
		wprintf_s(L"\n\n===================================\n\n");
		wprintf_s(L"Processor:%f / Process:%f \n", CPUTime.ProcessorTotal(), CPUTime.ProcessTotal());
		wprintf_s(L"ProcessorKernel:%f / ProcessKernel:%f \n", CPUTime.ProcessorKernel(), CPUTime.ProcessKernel());
		wprintf_s(L"ProcessorUser:%f / ProcessUser: %f \n", CPUTime.ProcessorUser(), CPUTime.ProcessUser());
		wprintf_s(L"ProcessUserMemory:%lld\n", CPUTime.ProcessUserMemory());
		wprintf_s(L"ProcessNonPagedMemory:%lld\n", CPUTime.ProcessNonPagedMemory());
		wprintf_s(L"AvailableMemory:%lld\n", CPUTime.AvailableMemory());
		wprintf_s(L"NonPagedMemory:%lld\n", CPUTime.NonPagedMemory());
		wprintf_s(L"ProcessHandleCount:%d\n", CPUTime.ProcessHandleCount());
		wprintf_s(L"ProcessThreadCount:%d\n", CPUTime.ProcessThreadCount());
		wprintf_s(L"NetworkRecvBytes:%lld\n", CPUTime.NetworkRecvBytes());
		wprintf_s(L"NetworkSendBytes:%lld\n", CPUTime.NetworkSendBytes());
		
		Sleep(1000);
	}

	return 0;
}