#include "MyProfiler.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "CSVReader.h"

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
	CSVFile csv(L"settings.csv");

	Setting setting;

	csv.NextColumn();

	for (int i = 0; i < COLUMN_SIZE; ++i)
	{
		memset(setting.colNames[i], 0, NAME_MAX * 2);
		csv.GetColumn(setting.colNames[i], NAME_MAX);
		csv.NextColumn();
	}

	csv.GetColumn(&setting.totalSize);
	csv.NextColumn();

	for (int i = 0; i < COLUMN_SIZE; ++i)
	{
		csv.GetColumn(&setting.colSize[i]);
		csv.NextColumn();
	}

	FILE* fout;
	tm t;
	time_t newTime;

	time(&newTime);
	localtime_s(&t, &newTime);
	WCHAR fileName[FILE_NAME_MAX];

	swprintf_s(fileName, _countof(fileName), L"%s_%04d%02d%02d_%02d%02d%02d.txt",
		szFileName,
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec);		

	_wfopen_s(&fout, fileName, L"a");

	WCHAR* div = new WCHAR[setting.totalSize];
	wmemset(div, L'-', setting.totalSize);
	div[setting.totalSize - 2] = L'\n';
	div[setting.totalSize - 1] = L'\0';

	fwprintf_s(fout, L"%ls\n", div);

	WCHAR* tableName = new WCHAR[setting.totalSize];
	WCHAR tableSet[FILE_NAME_MAX];
	swprintf_s(tableSet, FILE_NAME_MAX, L"%%%dls%%%dls%%%dls%%%dls%%%dls",
		setting.colSize[0], 
		setting.colSize[1], 
		setting.colSize[2], 
		setting.colSize[3], 
		setting.colSize[4]);

	swprintf_s(tableName, setting.totalSize, tableSet,
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
		double avg;
		double freq = f.QuadPart / 1000000.0;
		
		if (gProfiles[i].iCall > 2)
		{
			for (int j = 0; j < 2; ++j)
			{
				time -= gProfiles[i].iMax[i];
				time -= gProfiles[i].iMin[i];
			}

			avg = time / freq / (gProfiles[i].iCall - 4);
		}
		else
		{
			avg = time / freq / (gProfiles[i].iCall);
		}
		
		

		WCHAR nameTxt[32];
		WCHAR avgTxt[32];
		WCHAR minTxt[32];
		WCHAR maxTxt[32];
		WCHAR callTxt[32];

		swprintf_s(nameTxt, _countof(nameTxt), L"%s |", gProfiles[i].szName);
		swprintf_s(avgTxt, _countof(avgTxt), L"%.4lfus |", avg);
		swprintf_s(minTxt, _countof(minTxt), L"%.4lfus |", gProfiles[i].iMin[0] / freq);
		swprintf_s(maxTxt, _countof(maxTxt), L"%.4lfus |", gProfiles[i].iMax[0] / freq);
		swprintf_s(callTxt, _countof(callTxt), L"%lld |", gProfiles[i].iCall);

		swprintf_s(line, _countof(line), tableSet,
			nameTxt,
			avgTxt,
			minTxt,
			maxTxt,
			callTxt);

		fwprintf_s(fout, L"%ls\n", line);
	}
	fwprintf_s(fout, L"\n");
	fwprintf_s(fout, L"%ls", div);

	fclose(fout);

	delete[] div;
	delete[] tableName;
}

void ProfilePrint()
{
	for (int i = 1; i < PROFILE_MAX; ++i)
	{
		if (gProfiles[i].bFlag == false)
			break;

		LARGE_INTEGER f;
		WCHAR line[200];

		QueryPerformanceFrequency(&f);
		__int64 time = gProfiles[i].iTotalTime;
		double avg;
		//double freq = f.QuadPart / 1000000.0;  // us
		double freq = f.QuadPart / 1000.0;  // ms

		wprintf_s(L"%s : %.3lfms\n", gProfiles[i].szName, time / freq);
	}
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