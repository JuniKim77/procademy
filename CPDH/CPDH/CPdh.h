#pragma once

#pragma comment(lib, "Pdh.lib")

#include <stdio.h>
#include <Pdh.h>

class CPdh
{
public:
	CPdh(const WCHAR* processName);


private:
	PDH_HQUERY		mCpuQuery;
	WCHAR			mProcessname[MAX_PATH]; // NAME
	PDH_HCOUNTER	mCpuTotal; // "\\Processor(_Total)\\% Processor Time"
	PDH_HCOUNTER	mProcessUserMemory; // "\\Process(NAME)\\Private Bytes"
	PDH_HCOUNTER	mProcessNonPagedMemory; // "\\Process(NAME)\\Pool Nonpaged Bytes"
	PDH_HCOUNTER	mAvailableMemory; // L"\\Memory\\Available MBytes"
	PDH_HCOUNTER	mNonPagedMemory; // L"\\Memory\\Pool Nonpaged Bytes"
};