#include <Windows.h>
#include <tchar.h>

#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
#define PRO_END(TagName)	ProfileEnd(TagName)
#define PROFILE_MAX (50)
#define NAME_MAX (20)
#define FILE_NAME_MAX (80)
#define COLUMN_SIZE (6)

typedef struct
{
	bool			bFlag;				// ���������� ��� ����. (�迭�ÿ���)
	WCHAR			szName[NAME_MAX];	// �������� ���� �̸�.

	LARGE_INTEGER	lStartTime;			// �������� ���� ���� �ð�.

	__int64			iTotalTime;			// ��ü ���ð� ī���� Time.	(��½� ȣ��ȸ���� ������ ��� ����)
	__int64			iMin[2];			// �ּ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ּ� [1] ���� �ּ� [2])
	__int64			iMax[2];			// �ִ� ���ð� ī���� Time.	(�ʴ����� ����Ͽ� ���� / [0] �����ִ� [1] ���� �ִ� [2])

	__int64			iCall;				// ���� ȣ�� Ƚ��.
	int				iDepth;				// ����

} PROFILE_SAMPLE;

struct Setting
{
	WCHAR colNames[COLUMN_SIZE][32];
	int colSize[COLUMN_SIZE];
	int totalSize;
};

// extern PROFILE_SAMPLE gProfiles[PROFILE_MAX];

/////////////////////////////////////////////////////////////////////////////
// �ϳ��� �Լ� Profiling ����, �� �Լ�.
//
// Parameters: (char *)Profiling�̸�.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const WCHAR* szName);
void ProfileEnd(const WCHAR* szName);
void ProfileSetDepth(int depth, const WCHAR* szName);

/////////////////////////////////////////////////////////////////////////////
// Profiling �� ����Ÿ�� Text ���Ϸ� ����Ѵ�.
//
// Parameters: (char *)��µ� ���� �̸�.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const WCHAR* szFileName);

/////////////////////////////////////////////////////////////////////////////
// �������ϸ� �� �����͸� ��� �ʱ�ȭ �Ѵ�.
//
// Parameters: ����.
// Return: ����.
/////////////////////////////////////////////////////////////////////////////
void ProfileReset(void);

int SearchName(const WCHAR* s);