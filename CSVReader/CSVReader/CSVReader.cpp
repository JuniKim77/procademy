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
		delete[] (mpBuffer - 1);
	}
}

void CSVFile::readFile()
{
	FILE* fin;

	// UTF-8 이라도 읽고 나면 UTF-16LE 로 바뀜
	_wfopen_s(&fin, mFileName, L"r,ccs=UTF-8");

	fseek(fin, 0, SEEK_END);
	// 이 사이즈는 UTF-8 기준이기 때문에 실제 필요한 바이트 보다 작다.
	int size = ftell(fin);
	mpBuffer = new WCHAR[size + 2];
	memset(mpBuffer, 0, (size + 2) * 2);
	mpBuffer++;

	fseek(fin, 0, SEEK_SET);
	int result = fread_s(mpBuffer, (size + 1) * 2, (size + 1) * 2, 1, fin);

	mpCurrent = mpBuffer;

	countRow();
	countCol();

	fclose(fin);
}

bool CSVFile::SelectRow(int x)
{
	if (x < 0 || x >= mRowSize)
	{
		// log
		return false;
	}

	mpCurrent = mpBuffer;

	int rowCount = 0;
	
	while (rowCount < x)
	{
		while (1)
		{
			if (*mpCurrent == L'\n')
			{
				++mpCurrent;
				break;
			}

			++mpCurrent;
		}

		rowCount++;
	}

	return true;
}

bool CSVFile::NextRow()
{
	int count = 0;

	while (1)
	{
		if (*mpCurrent == L'\0')
		{
			return false;
		}

		if (count == mColSize)
		{
			break;
		}

		if (*mpCurrent == L',' || *mpCurrent == L'\n')
		{
			count++;
		}

		++mpCurrent;
	}

	return true;
}

bool CSVFile::PreRow()
{
	int count = 0;

	while (1)
	{
		if (count == mColSize)
		{
			while (1)
			{
				if (*mpCurrent == L',' || *mpCurrent == L'\0')
				{
					++mpCurrent;
					break;
				}
				--mpCurrent;
			}
			break;
		}

		if (*mpCurrent == L'\0')
		{
			return false;
		}

		if (*mpCurrent == L',' || *mpCurrent == L'\n')
		{
			count++;
		}

		--mpCurrent;
	}

	return true;
}

bool CSVFile::MoveColumn(int x)
{
	if (x < 0 || x >= mColSize)
		return false;

	while (1)
	{
		if (*mpCurrent == L'\n' || *mpCurrent == L'\0')
		{
			++mpCurrent;
			break;
		}
		--mpCurrent;
	}

	int count = 0;

	while (count < x)
	{
		if (*mpCurrent == L',')
		{
			count++;
		}

		++mpCurrent;
	}

	return true;
}

bool CSVFile::NextColumn()
{
	while (1)
	{
		if (*mpCurrent == L',' || *mpCurrent == L'\n')
		{
			++mpCurrent;
			break;
		}

		if (*mpCurrent == L'\0')
		{
			return false;
		}

		++mpCurrent;
	}

	return true;
}

bool CSVFile::PreColumn()
{
	if (mpCurrent == mpBuffer)
	{
		return false;
	}

	mpCurrent -= 2;

	while (1)
	{
		if (*mpCurrent == L',' || *mpCurrent == L'\n')
		{
			++mpCurrent;
			break;
		}

		if (*mpCurrent == L'\0')
		{
			++mpCurrent;
			break;
		}

		--mpCurrent;
	}

	return true;
}

int CSVFile::GetColumn(WCHAR* chValue, int size)
{
	if (size <= 0)
		return 0;

	WCHAR* pBuf = mpCurrent;
	int len = 0;

	while (1)
	{
		if (*pBuf == L',' || *pBuf == L'\n' || *pBuf == L'\0')
		{
			break;
		}
		++len;
		++pBuf;
	}

	if (len == 0)
		return 0;

	pBuf = mpCurrent;
	int count = 0;
	WCHAR* dest = chValue;

	if (size <= len)
	{
		while (count < size)
		{
			*dest++ = *pBuf++;
			count++;
		}

		return size - 1;
	}
	
	while (count < len)
	{
		*dest++ = *pBuf++;
		count++;
	}

	return len;
}

bool CSVFile::GetColumn(int* iValue)
{
	if (*mpCurrent == L',')
		return false;

	WCHAR* pBuf = mpCurrent;
	int len = 0;

	while (1)
	{
		if (*pBuf == L',' || *pBuf == L'\n' || *pBuf == L'\0')
		{
			break;
		}
		++len;
		++pBuf;
	}

	pBuf = mpCurrent;
	*iValue = _wtoi(pBuf);

	return true;
}

void CSVFile::countRow()
{
	const WCHAR* pBuf = mpBuffer;
	int count = 0;

	while (*pBuf != L'\0')
	{
		if (*pBuf == L'\n')
		{
			++count;
		}

		++pBuf;
	}

	mRowSize = count + 1;
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
