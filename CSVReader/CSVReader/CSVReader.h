#pragma once

#include <wtypes.h>

class CSVFile
{
public:
	CSVFile(const WCHAR* fileName);
	~CSVFile();
	int GetRow() { return mRowSize; }
	int GetCol() { return mColSize; }
	void SelectRow(int x);
	void NextRow();
	void PreRow();
	void MoveColumn(int x);
	void NextColumn();
	void PreColumn();
	void GetColumn(char* chValue);
	void GetColumn(short* shValue);
	void GetColumn(int* iValue);
	void GetColumn(__int64* iiValue);
	void GetColumn(float* fValue);
	void GetColumn(double* dValue);

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