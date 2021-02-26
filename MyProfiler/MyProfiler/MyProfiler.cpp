#include "MyProfiler.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

PROFILE_SAMPLE gProfiles[PROFILE_MAX];

void ProfileBegin(const WCHAR* szName)
{
	int idx = SearchName(szName);

	// 빈공간이 없는 경우
	if (idx == -1)
		return;

	if (gProfiles[idx].bFlag == false)
	{
		gProfiles[idx].bFlag = true;
		wcscat_s(gProfiles[idx].szName, _countof(gProfiles[idx].szName), szName);
	}

	QueryPerformanceCounter(&gProfiles[idx].lStartTime);
}

void ProfileEnd(const WCHAR* szName)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	int idx = SearchName(szName);

	__int64 time = end.QuadPart - gProfiles[idx].lStartTime.QuadPart;

	gProfiles[idx].iTotalTime += time;
	++gProfiles[idx].iCall;
	
	// Min Max 처리
	if (gProfiles[idx].iMin[1] > time)
	{
		gProfiles[idx].iMin[1] = time;
	}
	if (gProfiles[idx].iMax[1] < time)
	{
		gProfiles[idx].iMax[1] = time;
	}

	if (gProfiles[idx].iMax[0] < gProfiles[idx].iMax[1])
	{
		__int64 temp = gProfiles[idx].iMax[0];
		gProfiles[idx].iMax[0] = gProfiles[idx].iMax[1];
		gProfiles[idx].iMax[1] = temp;
	}
	if (gProfiles[idx].iMin[0] > gProfiles[idx].iMin[1])
	{
		__int64 temp = gProfiles[idx].iMin[0];
		gProfiles[idx].iMin[0] = gProfiles[idx].iMin[1];
		gProfiles[idx].iMin[1] = temp;
	}
}

void ProfileDataOutText(const WCHAR* szFileName)
{
	FILE* fout;
	tm t;
	time_t newTime;

	time(&newTime);
	localtime_s(&t, &newTime);
	WCHAR fileName[NAME_MAX];

	swprintf_s(fileName, _countof(fileName), L"%s_%04d%02d%02d_%02d%02d%02d.txt",
		szFileName,
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec);		

	_wfopen_s(&fout, fileName, L"a");

	WCHAR div[NAME_MAX];
	wmemset(div, L'-', NAME_MAX);
	div[NAME_MAX - 2] = L'\n';
	div[NAME_MAX - 1] = L'\0';

	fwprintf_s(fout, L"%ls\n", div);

	WCHAR tableName[NAME_MAX];
	swprintf_s(tableName, _countof(tableName), L"%20ls%15ls%15ls%15ls%13ls",
		L"Name  |",
		L"Average  |",
		L"Min   |",
		L"Max   |",
		L"Call   |");

	fwprintf_s(fout, L"%ls\n", tableName);
	fwprintf_s(fout, L"%ls", div);

	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (gProfiles[i].bFlag == false)
			break;

		LARGE_INTEGER f;
		WCHAR line[200];
		
		QueryPerformanceFrequency(&f);
		__int64 time = gProfiles[i].iTotalTime;
		
		if (gProfiles[i].iCall > 2)
		{
			for (int j = 0; j < 2; ++j)
			{
				time -= gProfiles[i].iMax[i];
				time -= gProfiles[i].iMin[i];
			}
		}
		double freq = f.QuadPart / 1000000.0;
		double avg = time / freq / (gProfiles[i].iCall - 2);

		swprintf_s(line, _countof(line), L"%20ls|%10.3f|%10.3lf|%10.3lf|%10.3lld",
			gProfiles[i].szName,
			avg,
			gProfiles[i].iMin[0] / freq,
			gProfiles[i].iMax[0] / freq,
			gProfiles[i].iCall);

		fwprintf_s(fout, L"%ls\n", line);
	}

	fclose(fout);
}

void ProfileReset(void)
{
	memset(gProfiles, 0, sizeof(gProfiles));
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		gProfiles[i].iMin[0] = MAXINT64;
		gProfiles[i].iMin[1] = MAXINT64;
	}
}

int SearchName(const WCHAR* s)
{
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (gProfiles[i].bFlag == false)
		{
			return i;
		}
		if (wcscmp(gProfiles[i].szName, s) == 0)
		{
			return i;
		}
	}

	return -1;
}
