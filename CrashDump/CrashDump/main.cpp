#include "CCrashDump.h"

int main()
{
	procademy::CCrashDump::CCrashDump();

	printf_s("Hello World");

	CRASH();

	return 0;
}