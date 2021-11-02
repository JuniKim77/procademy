#include "CCpuUsage.h"
#include <stdio.h>

int main()
{
	procademy::CCpuUsage CPUTime;

	while (1)
	{
		CPUTime.UpdateCpuTime();
		wprintf_s(L"Processor:%f / Process:%f \n", CPUTime.ProcessorTotal(), CPUTime.ProcessTotal());
		wprintf_s(L"ProcessorKernel:%f / ProcessKernel:%f \n", CPUTime.ProcessorKernel(), CPUTime.ProcessKernel());
		wprintf_s(L"ProcessorUser:%f / ProcessUser: %f \n", CPUTime.ProcessorUser(), CPUTime.ProcessUser());
		Sleep(1000);
	}

	return 0;
}