#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

int main()
{
	//LARGE_INTEGER Start;
	//LARGE_INTEGER End;
	//LARGE_INTEGER Freq;
	//QueryPerformanceFrequency(&Freq);	// 1초의 진동주기

	//QueryPerformanceCounter(&Start);
	//Sleep(1000);
	//QueryPerformanceCounter(&End);

	//printf("%f\n", (End.QuadPart - Start.QuadPart) / (double)Freq.QuadPart * 1000000);

	return 0;
}