#pragma once

#define PROFILE_MAX (50)
#define NAME_MAX (20)
#define FILE_NAME_MAX (80)
#define COLUMN_SIZE (7)

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
	static void InitProfiler(int num);
	static void DestroyProfiler();
	static void Begin(const WCHAR* szName);
	static void End(const WCHAR* szName);
	static void Print();
	static void SetChunk(int i) { s_chunk_size = i; }

private:
	int SearchName(const WCHAR* s);

public:
	static CProfiler**		s_profilers;
	static DWORD			s_MultiProfiler;
	static LONG				s_ProfilerIndex;
	static SRWLOCK			s_lock;
	static int				s_chunk_size;

private:
	typedef struct
	{
		bool			bFlag;				// ���������� ��� ����. (�迭�ÿ���)
		WCHAR			szName[NAME_MAX];	// �������� ���� �̸�.

		LARGE_INTEGER	lStartTime;			// �������� ���� ���� �ð�.

		__int64			iTotalTime;			// ��ü ���ð� ī���� Time.	(��½� ȣ��ȸ���� ������ ��� ����)
		__int64			iMin[2];			// �ּ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
		__int64			iMax[2];			// �ִ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])

		__int64			iCall;				// ���� ȣ�� Ƚ��.

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