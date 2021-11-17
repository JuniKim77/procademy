#include "CDBWriter.h"
#include "CProfiler.h"
#include "CCrashDump.h"
#include "CLogger.h"

int main()
{
	CLogger::SetDirectory(L"_log");
	CProfiler::InitProfiler(10);
	procademy::CCrashDump::SetHandlerDump();

	procademy::CDBWriter db;

	return 0;
}