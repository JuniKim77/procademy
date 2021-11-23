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
	/// ���� Row�� ù��° Col�� �̵�
	/// </summary>
	/// <param name="x"></param>
	bool SelectRow(int x);
	/// <summary>
	/// ���� ���� ���� ���� �ش� col�� �̵�
	/// </summary>
	bool NextRow();
	/// <summary>
	/// ���� ���� ���� ���� �ش� col�� �̵�
	/// </summary>
	bool PreRow();
	/// <summary>
	/// ���� ���� x ��° col�� �̵�
	/// </summary>
	/// <param name="x"></param>
	bool MoveColumn(int x);
	/// <summary>
	/// ���� ���� ���� col�� �̵�
	/// </summary>
	bool NextColumn();
	/// <summary>
	/// ���� ���� ���� col�� �̵�
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
