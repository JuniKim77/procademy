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
	PDH_HQUERY cpuQuery;
	PdhOpenQuery(NULL, NULL, &cpuQuery);

	// PDH ���ҽ� ī���� ���� (������ ������ �̸� ������ ����)
	PDH_HCOUNTER cpuTotal;
	PDH_HCOUNTER cpu1;
	PDH_HCOUNTER cpu2;
	PDH_HCOUNTER cpu3;
	PDH_HCOUNTER cpu4;
	PDH_HCOUNTER cpu5;
	PDH_HCOUNTER cpu6;

	PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	PdhAddCounter(cpuQuery, L"\\Processor(0)\\% Processor Time", NULL, &cpu1);
	PdhAddCounter(cpuQuery, L"\\Processor(1)\\% Processor Time", NULL, &cpu2);
	PdhAddCounter(cpuQuery, L"\\Processor(2)\\% Processor Time", NULL, &cpu3);
	PdhAddCounter(cpuQuery, L"\\Processor(3)\\% Processor Time", NULL, &cpu4);
	PdhAddCounter(cpuQuery, L"\\Processor(4)\\% Processor Time", NULL, &cpu5);
	PdhAddCounter(cpuQuery, L"\\Processor(5)\\% Processor Time", NULL, &cpu6);


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
		PdhGetFormattedCounterValue(cpu1, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_1 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu2, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_2 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu3, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_3 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu4, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_4 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu5, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_5 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu6, PDH_FMT_DOUBLE, NULL, &counterVal);
		// ���� ������ ���
		wprintf(L"CPU_6 Usage : %f%%\n", counterVal.doubleValue);
	}

	return 0;
}