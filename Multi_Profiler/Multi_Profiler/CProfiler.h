#pragma once

#define PROFILE_MAX (100)
#define NAME_MAX (32)
#define FILE_NAME_MAX (80)
#define MAX_PARSER_LENGTH (256)

#include <wtypes.h>

class CProfiler
{
public:
	enum class PROFILE_TYPE {
		MICRO_SECONDS,
		PERCENT,
		COUNT,
	};
public:
	CProfiler(const WCHAR* szSettingFileName);
	~CProfiler();

private:
	/// <summary>
	/// Profile Init
	/// CSV File Load
	/// </summary>
	/// <param name="szSettingFileName">Setting File Name</param>
	void ProfileInitialize(const WCHAR* szSettingFileName);
	/// <summary>
	/// Profile Begin
	/// </summary>
	/// <param name="szName">Profiling Name</param>
	void ProfileBegin(const WCHAR* szName);
	/// <summary>
	/// Profile End
	/// </summary>
	/// <param name="szName">Profiling Name</param>
	void ProfileEnd(const WCHAR* szName);
	void ProfileSetRecord(const WCHAR* szName, LONGLONG data, PROFILE_TYPE type);
	/// <summary>
	/// Profiling Data Text Out
	/// </summary>
	/// <param name="szFileName">File Name</param>
	void ProfileDataOutText(const WCHAR* szFileName);
	/// <summary>
	/// Current Profiling Data Logging to Console
	/// </summary>
	void ProfilePrint();
	/// <summary>
	/// Profile Reset
	/// </summary>
	void ProfileReset();

	void SetThreadId();

public:
	static void SetProfileFileName(WCHAR* szFileName);
	static void InitProfiler(int num);
	static void DestroyProfiler();
	static void Begin(const WCHAR* szName);
	static void End(const WCHAR* szName);
	static void SetRecord(const WCHAR* szName, LONGLONG data, PROFILE_TYPE type);
	static void Print();
	static void PrintAvg();

private:
	int SearchName(const WCHAR* s);

public:
	static CProfiler** s_profilers;
	static DWORD s_MultiProfiler;
	static LONG s_ProfilerIndex;
	static SRWLOCK s_lock;

private:
	typedef struct
	{
		bool			bFlag;				// 프로파일의 사용 여부. (배열시에만)
		PROFILE_TYPE	type;				// 데이터 타입
		WCHAR			szName[NAME_MAX];	// 프로파일 샘플 이름.

		LARGE_INTEGER	lStartTime;			// 프로파일 샘플 실행 시간.

		__int64			iTotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
		__int64			iMin[2];			// 최소 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
		__int64			iMax[2];			// 최대 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])

		__int64			iCall;				// 누적 호출 횟수.

	} PROFILE_SAMPLE;

	struct Setting
	{
		WCHAR** colNames;
		int* colSize;
		int totalSize;
	};

	enum SEARCH_RESULT
	{
		SEARCH_RESULT_FULL = -1
	};

	PROFILE_SAMPLE mProfiles[PROFILE_MAX];
	Setting mSetting;
	DWORD mThreadId = 0;
	int mColumnSize = 0;
};
