#include "CCpuUsage.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int main()
{
	DWORD process_id = GetCurrentProcessId();
	HANDLE process_handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		process_id
	);

	if (process_handle) {
		wchar_t buffer[MAX_PATH] = {};
		DWORD buffer_size = MAX_PATH;
		if (QueryFullProcessImageNameW(process_handle, 0, buffer, &buffer_size)) {
			std::wcout << L"QueryFullProcessImageNameW process name : " << buffer << std::endl;
		}
		CloseHandle(process_handle);
	}

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