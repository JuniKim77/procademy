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

	// PDH 리소스 카운터 생성 (여러개 수집시 이를 여러개 생성)
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


	// 첫 갱신
	PdhCollectQueryData(cpuQuery);


	while (true)
	{
		Sleep(1000);

		// 1초마다 갱신
		PdhCollectQueryData(cpuQuery);

		// 갱신 데이터 얻음
		PDH_FMT_COUNTERVALUE counterVal;
		PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu1, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_1 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu2, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_2 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu3, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_3 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu4, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_4 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu5, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_5 Usage : %f%%\n", counterVal.doubleValue);
		PdhGetFormattedCounterValue(cpu6, PDH_FMT_DOUBLE, NULL, &counterVal);
		// 얻은 데이터 사용
		wprintf(L"CPU_6 Usage : %f%%\n", counterVal.doubleValue);
	}

	return 0;
}