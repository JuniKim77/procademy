#pragma once

#define PROFILE_MAX (50)
#define NAME_MAX (20)
#define FILE_NAME_MAX (80)
#define COLUMN_SIZE (6)

#include <wtypes.h>

class CProfiler
{
public:
	CProfiler(const WCHAR* szSettingFileName);
	~CProfiler();
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

	static void SetProfileFileName(WCHAR* szFileName);

private:
	int SearchName(const WCHAR* s);

private:
	typedef struct
	{
		bool			bFlag;				// 프로파일의 사용 여부. (배열시에만)
		WCHAR			szName[NAME_MAX];	// 프로파일 샘플 이름.

		LARGE_INTEGER	lStartTime;			// 프로파일 샘플 실행 시간.

		__int64			iTotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
		__int64			iMin[2];			// 최소 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
		__int64			iMax[2];			// 최대 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])

		__int64			iCall;				// 누적 호출 횟수.

	} PROFILE_SAMPLE;

	struct Setting
	{
		WCHAR colNames[COLUMN_SIZE][32];
		int colSize[COLUMN_SIZE];
		int totalSize;
	};

	enum SEARCH_RESULT
	{
		SEARCH_RESULT_FULL = -1
	};
	
	PROFILE_SAMPLE mProfiles[PROFILE_MAX];
	Setting mSetting;
	DWORD mThreadId = 0;
};