#include <Windows.h>
#include <stdio.h>
#include "CDebugger.h"

int main()
{
	printf("%u\n", GetCurrentThreadId());
	CDebugger::_Log(GetCurrentThreadId(), L"");

	return 0;
}