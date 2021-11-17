#include "CProfiler.h"
#include <stdio.h>
#include <time.h>
#include "CSVReader.h"
#include <string.h>
#include <wchar.h>

CProfiler** CProfiler::s_profilers;
LONG CProfiler::s_ProfilerIndex = 0;
DWORD CProfiler::s_MultiProfiler;
SRWLOCK CProfiler::s_lock;
int	CProfiler::s_chunk_size;

CProfiler::CProfiler(const WCHAR* szmSettingFileName)
{
	ProfileInitialize(szmSettingFileName);
	ProfileReset();
}

CProfiler::~CProfiler()
{
	wprintf_s(L"Exit CProfiler\n");
}

void CProfiler::ProfileInitialize(const WCHAR* szmSettingFileName)
{
	CSVFile csv(szmSettingFileName);

	for (int i = 0; i < COLUMN_SIZE; ++i)
	{
		memset(mSetting.colNames[i], 0, NAME_MAX * 2);
		csv.GetColumn(mSetting.colNames[i], NAME_MAX);
		csv.NextColumn();
		wcscat_s(mSetting.colNames[i], 32, L"  |");
	}

	mSetting.totalSize = 2;

	for (int i = 0; i < COLUMN_SIZE; ++i)
	{
		csv.GetColumn(&mSetting.colSize[i]);
		csv.NextColumn();

		mSetting.totalSize += mSetting.colSize[i];
	}
}

void CProfiler::ProfileBegin(const WCHAR* szName)
{
	int idx = SearchName(szName);

	if (idx == SEARCH_RESULT_FULL)
	{
		wprintf_s(L"Profiling List is Full\n");

		return;
	}

	if (mProfiles[idx].bFlag == false)
	{
		mProfiles[idx].bFlag = true;
		wcscat_s(mProfiles[idx].szName, _countof(mProfiles[idx].szName), szName);
	}

	QueryPerformanceCounter(&mProfiles[idx].lStartTime);
}

void CProfiler::ProfileEnd(const WCHAR* szName)
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	int idx = SearchName(szName);

	if (idx == SEARCH_RESULT_FULL)
	{
		wprintf_s(L"Profiling List is Full\n");

		return;
	}

	__int64 time = end.QuadPart - mProfiles[idx].lStartTime.QuadPart;

	mProfiles[idx].iTotalTime += time;
	++mProfiles[idx].iCall;

	// Min Max Ã³¸®
	if (mProfiles[idx].iMin[1] > time)
	{
		mProfiles[idx].iMin[1] = time;
	}
	if (mProfiles[idx].iMax[1] < time)
	{
		mProfiles[idx].iMax[1] = time;
	}

	if (mProfiles[idx].iMax[0] < mProfiles[idx].iMax[1])
	{
		__int64 temp = mProfiles[idx].iMax[0];
		mProfiles[idx].iMax[0] = mProfiles[idx].iMax[1];
		mProfiles[idx].iMax[1] = temp;
	}
	if (mProfiles[idx].iMin[0] > mProfiles[idx].iMin[1])
	{
		__int64 temp = mProfiles[idx].iMin[0];
		mProfiles[idx].iMin[0] = mProfiles[idx].iMin[1];
		mProfiles[idx].iMin[1] = temp;
	}
}

void CProfiler::ProfileDataOutText(const WCHAR* szFileName)
{
	FILE* fout;

	_wfopen_s(&fout, szFileName, L"a");

	WCHAR* div = new WCHAR[mSetting.totalSize];
	wmemset(div, L'-', mSetting.totalSize);
	div[mSetting.totalSize - 2] = L'\n';
	div[mSetting.totalSize - 1] = L'\0';

	fwprintf_s(fout, L"%ls", div);

	WCHAR* tableName = new WCHAR[mSetting.totalSize];
	WCHAR tableSet[FILE_NAME_MAX];
	swprintf_s(tableSet, FILE_NAME_MAX, L"%%%dls%%%dls%%%dls%%%dls%%%dls%%%dls%%%dls",
		mSetting.colSize[0],
		mSetting.colSize[1],
		mSetting.colSize[2],
		mSetting.colSize[3],
		mSetting.colSize[4],
		mSetting.colSize[5],
		mSetting.colSize[6]);

	swprintf_s(tableName, mSetting.totalSize, tableSet,
		mSetting.colNames[0],
		mSetting.colNames[1],
		mSetting.colNames[2],
		mSetting.colNames[3],
		mSetting.colNames[4],
		mSetting.colNames[5],
		mSetting.colNames[6]);

	fwprintf_s(fout, L"%ls\n", tableName);
	fwprintf_s(fout, L"%ls", div);

	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (mProfiles[i].bFlag == false)
			break;

		LARGE_INTEGER f;
		WCHAR line[200];

		QueryPerformanceFrequency(&f);
		__int64 time = mProfiles[i].iTotalTime;
		double avg;
		double freq = f.QuadPart / 1000000.0;

		if (mProfiles[i].iCall > 2)
		{
			for (int j = 0; j < 2; ++j)
			{
				time -= mProfiles[i].iMax[j];
				time -= mProfiles[i].iMin[j];
			}

			avg = time / freq / (mProfiles[i].iCall - 4);
		}
		else
		{
			avg = time / freq / (mProfiles[i].iCall);
		}

		WCHAR threadIdTxt[32];
		WCHAR nameTxt[32];
		WCHAR avgTxt[32];
		WCHAR minTxt[32];
		WCHAR maxTxt[32];
		WCHAR callTxt[32];
		WCHAR chunkTxt[32];

		swprintf_s(threadIdTxt, _countof(threadIdTxt), L"%u |", mThreadId);
		swprintf_s(nameTxt, _countof(nameTxt), L"%s |", mProfiles[i].szName);
		swprintf_s(avgTxt, _countof(avgTxt), L"%.4lfus |", avg);
		swprintf_s(minTxt, _countof(minTxt), L"%.4lfus |", mProfiles[i].iMin[0] / freq);
		swprintf_s(maxTxt, _countof(maxTxt), L"%.4lfus |", mProfiles[i].iMax[0] / freq);
		swprintf_s(callTxt, _countof(callTxt), L"%lld |", mProfiles[i].iCall);
		swprintf_s(chunkTxt, _countof(chunkTxt), L"%d |", CProfiler::s_chunk_size);

		swprintf_s(line, _countof(line), tableSet,
			threadIdTxt,
			nameTxt,
			avgTxt,
			minTxt,
			maxTxt,
			callTxt,
			chunkTxt);

		fwprintf_s(fout, L"%ls\n", line);
	}
	fwprintf_s(fout, L"%ls\n", div);

	fclose(fout);

	delete[] div;
	delete[] tableName;
}

void CProfiler::ProfilePrint()
{
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (mProfiles[i].bFlag == false)
			break;

		LARGE_INTEGER f;

		QueryPerformanceFrequency(&f);
		__int64 time = mProfiles[i].iTotalTime;
		double avg;
		double freq = f.QuadPart / 1000.0;  // ms

		wprintf_s(L"%s : %.3lfms\n", mProfiles[i].szName, time / freq);
	}
}

void CProfiler::ProfileReset()
{
	memset(mProfiles, 0, sizeof(mProfiles));
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		mProfiles[i].iMin[0] = MAXINT64;
		mProfiles[i].iMin[1] = MAXINT64;
	}
}

void CProfiler::SetThreadId()
{
	mThreadId = GetCurrentThreadId();
}

void CProfiler::SetProfileFileName(WCHAR* szFileName)
{
	tm t;
	time_t newTime;

	time(&newTime);
	localtime_s(&t, &newTime);
	WCHAR fileName[FILE_NAME_MAX];

	/*swprintf_s(fileName, _countof(fileName), L"_%04d%02d%02d_%02d%02d%02d.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec);*/
	swprintf_s(fileName, _countof(fileName), L"_%04d%02d%02d.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday);

	wcscat_s(szFileName, FILE_NAME_MAX, fileName);
}

void CProfiler::InitProfiler(int num)
{
	s_MultiProfiler = TlsAlloc();
	InitializeSRWLock(&s_lock);
	s_profilers = new CProfiler * [num];

	for (int i = 0; i < num; ++i)
	{
		s_profilers[i] = new CProfiler(L"settings.csv");
	}
}

void CProfiler::DestroyProfiler()
{
	for (int i = 0; i < s_ProfilerIndex; ++i)
	{
		delete s_profilers[i];
	}

	delete[] s_profilers;
}

void CProfiler::Begin(const WCHAR* szName)
{
	CProfiler* profiler = (CProfiler*)TlsGetValue(s_MultiProfiler);

	if (profiler == nullptr)
	{
		AcquireSRWLockExclusive(&s_lock);
		profiler = s_profilers[s_ProfilerIndex++];
		TlsSetValue(s_MultiProfiler, profiler);
		profiler->SetThreadId();
		profiler->ProfileReset();
		ReleaseSRWLockExclusive(&s_lock);
	}

	profiler->ProfileBegin(szName);
}

void CProfiler::End(const WCHAR* szName)
{
	CProfiler* profiler = (CProfiler*)TlsGetValue(s_MultiProfiler);

	profiler->ProfileEnd(szName);
}

void CProfiler::Print()
{
	WCHAR fileName[FILE_NAME_MAX] = L"Profile";

	CProfiler::SetProfileFileName(fileName);

	for (int i = 0; i < s_ProfilerIndex; ++i)
	{
		s_profilers[i]->ProfileDataOutText(fileName);
		s_profilers[i]->ProfileReset();
	}
}

int CProfiler::SearchName(const WCHAR* s)
{
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (mProfiles[i].bFlag == false)
		{
			return i;
		}
		if (wcscmp(mProfiles[i].szName, s) == 0)
		{
			return i;
		}
	}

	return SEARCH_RESULT_FULL;
}
