#pragma warning(disable:4101)
#pragma warning(disable:6387)

#include "CProfilerClock.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <wchar.h>
#include <intrin.h>

procademy::CProfilerClock procademy::CProfilerClock::s_profilers[PROFILER_MAX];
LONG procademy::CProfilerClock::s_ProfilerIndex = 0;
DWORD procademy::CProfilerClock::s_MultiProfiler;
bool procademy::CProfilerClock::s_spinlock = false;

procademy::CProfilerClock::CProfilerClock()
{
	ProfileReset();
}

procademy::CProfilerClock::~CProfilerClock()
{
}

void procademy::CProfilerClock::ProfileBegin(const WCHAR* szName)
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

	mProfiles[idx].lStartTime = __rdtsc();
}

void procademy::CProfilerClock::ProfileEnd(const WCHAR* szName)
{
	unsigned __int64 end = __rdtsc();

	int idx = SearchName(szName);

	if (idx == SEARCH_RESULT_FULL)
	{
		wprintf_s(L"Profiling List is Full\n");

		return;
	}

	if (mProfiles[idx].lStartTime == 0)
	{
		return;
	}

	__int64 time = end - mProfiles[idx].lStartTime;

	mProfiles[idx].iTotalTime += time;
	++mProfiles[idx].iCall;

	// Min Max 贸府
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

void procademy::CProfilerClock::ProfileSetRecord(const WCHAR* szName, unsigned __int64 data)
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

	mProfiles[idx].iTotalTime += data;
	++mProfiles[idx].iCall;

	// Min Max 贸府
	if (mProfiles[idx].iMin[1] > data)
	{
		mProfiles[idx].iMin[1] = data;
	}
	if (mProfiles[idx].iMax[1] < data)
	{
		mProfiles[idx].iMax[1] = data;
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

void procademy::CProfilerClock::ProfileDataOutText(const WCHAR* szFileName)
{
	FILE* fout;

	_wfopen_s(&fout, szFileName, L"a");

	WCHAR div[126];
	wmemset(div, L'-', _countof(div));
	div[_countof(div) - 2] = L'\n';
	div[_countof(div) - 1] = L'\0';

	fwprintf_s(fout, L"%ls", div);

	WCHAR tableName[126];
	WCHAR tableSet[FILE_NAME_MAX] = L"%12s%25s%24s%24s%24s%15s";

	swprintf_s(tableName, _countof(tableName), tableSet, L"ThreadID  |", L"Name  |", L"Average  |", L"Min  |", L"Max  |", L"Call  |");

	fwprintf_s(fout, L"%ls\n", tableName);
	fwprintf_s(fout, L"%ls", div);

	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		if (mProfiles[i].bFlag == false)
			break;

		WCHAR line[200];

		unsigned __int64 time = mProfiles[i].iTotalTime;
		double avg;

		if (mProfiles[i].iCall > 4)
		{
			for (int j = 0; j < 2; ++j)
			{
				time -= mProfiles[i].iMax[j];
				time -= mProfiles[i].iMin[j];
			}

			avg = time / (mProfiles[i].iCall - 4);
		}
		else
		{
			avg = time / (mProfiles[i].iCall);
		}

		WCHAR threadIdTxt[32];
		WCHAR nameTxt[32];
		WCHAR avgTxt[32];
		WCHAR minTxt[32];
		WCHAR maxTxt[32];
		WCHAR callTxt[32];

		swprintf_s(threadIdTxt, _countof(threadIdTxt), L"%u |", mThreadId);
		swprintf_s(nameTxt, _countof(nameTxt), L"%s |", mProfiles[i].szName);
		swprintf_s(callTxt, _countof(callTxt), L"%lld |", mProfiles[i].iCall);
		swprintf_s(avgTxt, _countof(avgTxt), L"%.4lf |", avg);
		swprintf_s(minTxt, _countof(minTxt), L"%llu |", mProfiles[i].iMin[0]);
		swprintf_s(maxTxt, _countof(maxTxt), L"%llu |", mProfiles[i].iMax[0]);

		swprintf_s(line, _countof(line), tableSet,
			threadIdTxt,
			nameTxt,
			avgTxt,
			minTxt,
			maxTxt,
			callTxt);

		fwprintf_s(fout, L"%ls\n", line);
	}
	fwprintf_s(fout, L"%ls\n", div);

	fclose(fout);
}

void procademy::CProfilerClock::ProfileReset()
{
	memset(mProfiles, 0, sizeof(mProfiles));
	for (int i = 0; i < PROFILE_MAX; ++i)
	{
		mProfiles[i].iMin[0] = MAXUINT64;
		mProfiles[i].iMin[1] = MAXUINT64;
	}
}

void procademy::CProfilerClock::SetThreadId()
{
	mThreadId = GetCurrentThreadId();
}

int procademy::CProfilerClock::SearchName(const WCHAR* s)
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

void procademy::CProfilerClock::SetProfileFileName(WCHAR* szFileName)
{
	tm t;
	time_t newTime;

	time(&newTime);
	localtime_s(&t, &newTime);
	WCHAR fileName[FILE_NAME_MAX];

	swprintf_s(fileName, _countof(fileName), L"_%04d%02d%02d_%02d%02d%02d.txt",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec);

	wcscat_s(szFileName, FILE_NAME_MAX, fileName);
}

void procademy::CProfilerClock::Begin(const WCHAR* szName)
{
	CProfilerClock* profiler = (CProfilerClock*)TlsGetValue(s_MultiProfiler);

	if (profiler == nullptr)
	{
		while (InterlockedExchange8((char*)&s_spinlock, true) == true)
		{
		}

		profiler = &s_profilers[s_ProfilerIndex++];
		TlsSetValue(s_MultiProfiler, profiler);
		profiler->SetThreadId();

		s_spinlock = false;
	}

	profiler->ProfileBegin(szName);
}

void procademy::CProfilerClock::End(const WCHAR* szName)
{
	CProfilerClock* profiler = (CProfilerClock*)TlsGetValue(s_MultiProfiler);

	profiler->ProfileEnd(szName);
}

void procademy::CProfilerClock::SetRecord(const WCHAR* szName, unsigned __int64 data)
{
	CProfilerClock* profiler = (CProfilerClock*)TlsGetValue(s_MultiProfiler);

	if (profiler == nullptr)
	{
		while (InterlockedExchange8((char*)&s_spinlock, true) == true)
		{
		}

		profiler = &s_profilers[s_ProfilerIndex++];
		TlsSetValue(s_MultiProfiler, profiler);
		profiler->SetThreadId();

		s_spinlock = false;
	}

	profiler->ProfileSetRecord(szName, data);
}

void procademy::CProfilerClock::Print()
{
	WCHAR fileName[FILE_NAME_MAX] = L"Profile";

	CProfilerClock::SetProfileFileName(fileName);

	for (int i = 0; i < s_ProfilerIndex; ++i)
	{
		s_profilers[i].ProfileDataOutText(fileName);
		s_profilers[i].ProfileReset();
	}
}

void procademy::CProfilerClock::PrintAvg()
{
	WCHAR fileName[FILE_NAME_MAX] = L"Profile";

	CProfilerClock::SetProfileFileName(fileName);

	int curIndex = s_ProfilerIndex;

	for (int i = 0; i < curIndex; ++i)
	{
		for (int j = 0; j < PROFILE_MAX; ++j)
		{
			if (s_profilers[i].mProfiles[j].bFlag == false)
				break;

			LARGE_INTEGER f;

			QueryPerformanceFrequency(&f);
			__int64 time = s_profilers[i].mProfiles[j].iTotalTime;
			double avg;

			if (s_profilers[i].mProfiles[j].iCall > 4)
			{
				for (int t = 0; t < 2; ++t)
				{
					time -= s_profilers[i].mProfiles[j].iMax[t];
					time -= s_profilers[i].mProfiles[j].iMin[t];
				}

				avg = time / (s_profilers[i].mProfiles[j].iCall - 4);
			}
			else
			{
				avg = time / (s_profilers[i].mProfiles[j].iCall);
			}

			s_profilers[s_ProfilerIndex].SetRecord(s_profilers[i].mProfiles[j].szName, avg);
		}

		s_profilers[i].ProfileDataOutText(fileName);
		s_profilers[i].ProfileReset();
	}

	s_profilers[curIndex].ProfileDataOutText(fileName);
	s_profilers[curIndex].ProfileReset();
}
