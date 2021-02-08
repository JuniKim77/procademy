#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <chrono>

#pragma comment(lib, "winmm.lib")

#define D

int main()
{
#ifdef A
	while (1)
	{
		printf("Time: %d\n", clock()); // 1ms�� ����.
	}
#elif defined B
	while (1)
	{
		printf("Tick: %I64d\n", GetTickCount64()); // 16ms�� ����
	}
#elif defined C
	while (1)
	{
		printf("Time: %d\n", timeGetTime()); // 1ms�� ����.
	}
#elif defined D
	timeBeginPeriod(1);
	while (1)
	{
		printf("Time: %d\n", timeGetTime()); // 1ms�� ����.
		Sleep(1); // timeBeginPeriod�� �����Ǿ� ����? ����
		printf("Time: %d\n", timeGetTime()); // 1ms�� ����.
	}
	timeEndPeriod(1);
#elif defined E
	/*__int64 currentTime;
	__int64 gap[1000];

	int count = -1;
	int maxCount = 100;
	__int64 diff = 0;

	printf("QueryInterruptTimePrecise\n");
	while (count++ < maxCount)
	{
		QueryInterruptTime((PULONGLONG)&currentTime);
		gap[count] = currentTime;
		printf("%I64d\n", currentTime);
	}

	currentTime = gap[0];
	diff = 0;
	for (int i = 1; i < maxCount; i++)
	{
		diff = gap[i] - currentTime;
		printf("%I64d, %0.4f\n", diff, diff / 10000.0);
		currentTime = gap[i];
	}*/
#elif defined F
	std::chrono::system_clock::time_point time;

	while (1)
	{
		time = std::chrono::system_clock::now();
		printf("Time chrono: %I64d\n", time); // ns ���� �ε�.
	}
#elif defined G
	time_t t;
	while (1)
	{
		time(&t);
		printf("Time: %d\n", t); // 1�ʾ� ����.
	}
#endif

	return 0;
}