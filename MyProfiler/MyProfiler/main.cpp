#include <Windows.h>
#include <stdio.h>

int main()
{
	LARGE_INTEGER Start;
	LARGE_INTEGER End;
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);	// 1���� �����ֱ�

	QueryPerformanceCounter(&Start);
	Sleep(1000);
	QueryPerformanceCounter(&End);

	printf("%lld", End.QuadPart - Start.QuadPart);
	return 0;
}