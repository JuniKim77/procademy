#pragma once
#include "CSVReader.h"
#include <stdio.h>
#include <memory>

CSVFile::CSVFile(const WCHAR* fileName)
	: mFileName(fileName)
	, mpBuffer(nullptr)
{
	readFile();
}

CSVFile::~CSVFile()
{
	if (mpBuffer != nullptr)
	{
		delete[] mpBuffer;
	}
}

void CSVFile::readFile()
{
	FILE* fin;

	_wfopen_s(&fin, mFileName, L"r,ccs=UTF-16LE");

	fseek(fin, 0, SEEK_END);

	int size = ftell(fin);
	mpBuffer = new WCHAR[size];
#ifdef _DEBUG
	memset(mBuffer, 0, size);
#endif
	fseek(fin, 0, SEEK_SET);
	int result = fread_s(mpBuffer, size, size, 1, fin);

	mpCurrent = mpBuffer;

	countRow();
	countCol();

	fclose(fin);
}

void CSVFile::SelectRow(int x)
{
	if (x < 0 || x >= mRowSize)
		return;

	mpCurrent = mpBuffer;

	int rowCount = 0;
	
	while (rowCount < x) 
	{
		int colCount = 0;

		while (colCount < mColSize)
		{

		}
	}
}

void CSVFile::NextRow()
{
}

void CSVFile::PreRow()
{
}

void CSVFile::MoveColumn(int x)
{
}

void CSVFile::NextColumn()
{
}

void CSVFile::PreColumn()
{
}

void CSVFile::GetColumn(char* chValue)
{
}

void CSVFile::GetColumn(short* shValue)
{
}

void CSVFile::GetColumn(int* iValue)
{
}

void CSVFile::GetColumn(__int64* iiValue)
{
}

void CSVFile::GetColumn(float* fValue)
{
}

void CSVFile::GetColumn(double* dValue)
{
}

void CSVFile::countRow()
{
	const WCHAR* pBuf = mpBuffer;
	int count = 0;

	while (*pBuf != L'\0')
	{
		if (*pBuf == L'\n')
			++count;

		++pBuf;
	}

	mRowSize = count - 1;
}

void CSVFile::countCol()
{
	const WCHAR* pBuf = mpBuffer;
	int count = 0;

	while (*pBuf != L'\n')
	{
		if (*pBuf == L',')
			++count;

		++pBuf;
	}

	mColSize = count + 1;
}
