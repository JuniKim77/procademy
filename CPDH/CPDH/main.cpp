#define df_PDH_ETHERNET_MAX (8)

#pragma comment(lib, "Pdh.lib")

#include <wtypes.h>
#include <stdio.h>
#include <Pdh.h>

struct st_ETHERNET
{
	bool			bUse;
	WCHAR			szName[128];
};

int main()
{
	DWORD process_id = GetCurrentProcessId();
	HANDLE process_handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		process_id
	);
	wchar_t buffer[MAX_PATH] = {};
	if (process_handle) {
		
		DWORD buffer_size = MAX_PATH;
		if (QueryFullProcessImageNameW(process_handle, 0, buffer, &buffer_size)) {
			wprintf(L"QueryFullProcessImageNameW process name : %s\n", buffer);
		}
		CloseHandle(process_handle);
	}

	PDH_HQUERY cpuQuery;
	PdhOpenQuery(NULL, NULL, &cpuQuery);

	// PDH ���ҽ� ī���� ���� (������ ������ �̸� ������ ����)
	PDH_HCOUNTER cpuTotal;
	PDH_HCOUNTER cpu1;
	PDH_HCOUNTER cpu2;
	PDH_HCOUNTER processUserUsage;

	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhAddCounter(cpuQuery, L"\\Processor(0)\\% Processor Time", NULL, &cpu1);
	PdhAddCounter(cpuQuery, L"\\Processor(1)\\% Processor Time", NULL, &cpu2);
	PdhAddCounter(cpuQuery, L"\\Process(CPDH)\\Thread Count", NULL, &processUserUsage);


	// ù ����
	PdhCollectQueryData(cpuQuery);


	while (true)
	{
		Sleep(1000);

		// 1�ʸ��� ����
		PdhCollectQueryData(cpuQuery);

		// ���� ������ ����
		PDH_FMT_COUNTERVALUE counterVal;
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(processUserUsage, PDH_FMT_DOUBLE, NULL, &counterVal);
		
		wprintf(L"Process Handle Count : %f%%\n", counterVal.doubleValue);

	}

	return 0;
}