#pragma once

#include <wtypes.h>

class CSVFile
{
public:
	CSVFile(const WCHAR* fileName);
	~CSVFile();
	int GetRow() { return mRowSize; }
	int GetCol() { return mColSize; }
	/// <summary>
	/// 선택 Row의 첫번째 Col로 이동
	/// </summary>
	/// <param name="x"></param>
	bool SelectRow(int x);
	/// <summary>
	/// 현재 기준 다음 줄의 해당 col로 이동
	/// </summary>
	bool NextRow();
	/// <summary>
	/// 현재 기준 이전 줄의 해당 col로 이동
	/// </summary>
	bool PreRow();
	/// <summary>
	/// 현재 줄의 x 번째 col로 이동
	/// </summary>
	/// <param name="x"></param>
	bool MoveColumn(int x);
	/// <summary>
	/// 현재 기준 다음 col로 이동
	/// </summary>
	bool NextColumn();
	/// <summary>
	/// 현재 기준 이전 col로 이동
	/// </summary>
	bool PreColumn();
	int GetColumn(WCHAR* chValue, int size);
	bool GetColumn(int* iValue);

private:
	void readFile();
	void countRow();
	void countCol();

private:
	int mRowSize;
	int mColSize;
	const WCHAR* mFileName;
	WCHAR* mpBuffer;
	WCHAR* mpCurrent;
};
