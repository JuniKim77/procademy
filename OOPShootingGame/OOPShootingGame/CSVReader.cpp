#pragma once
#include "CSVReader.h"
#include "operatorNewOverload.h"
#include <stdio.h>
#include <memory>

CSVFile::CSVFile(const char* fileName)
	: mFileName(fileName)
	, mBuffer(nullptr)
{
	readFile();
}

CSVFile::~CSVFile()
{
	if (mBuffer != nullptr)
	{
		delete[] mBuffer;
	}
}

void CSVFile::readFile()
{
	FILE* fin;

	fopen_s(&fin, mFileName, "r");
	fseek(fin, 0, SEEK_END);

	int size = ftell(fin);
	mBuffer = new char[size];
	memset(mBuffer, 0, size);
	fseek(fin, 0, SEEK_SET);
	fread_s(mBuffer, size, size, 1, fin);

	mRowSize = countRow(mBuffer);
	mColSize = countCol(mBuffer);

	fclose(fin);
}

const char* CSVFile::GetRowAddress(int line)
{
	if (line < 1 || line > mRowSize)
		return nullptr;

	char* pBuf = mBuffer;
	int count = 0;

	while (count < line)
	{
		while (*pBuf != '\n')
		{
			++pBuf;
		}
		++pBuf;
		++count;
	}

	return pBuf;
}

const char* CSVFile::GetTitleAddress()
{
	char* pBuf = mBuffer;

	while (*pBuf < 0)
	{
		++pBuf;
	}

	return pBuf;
}

void CSVFile::GetNextComma(const char** pBuf)
{
	while (**pBuf != ',' && **pBuf != '\0')
	{
		++(*pBuf);
	}
}

bool CSVFile::CopyToNextComma(char* buffer, const char** pBegin)
{
	const char* pEnd = *pBegin;

	while (*pEnd != ',' && *pEnd != '\n' && *pEnd != '\0')
	{
		++pEnd;
	}

	int size = pEnd - *pBegin;

	if (size == 0)
	{
		*pBegin = pEnd + 1;
		return false;
	}
		
	memcpy(buffer, *pBegin, size);
	buffer[size] = '\0';

	*pBegin = pEnd + 1;

	return true;
}

int CSVFile::countRow(const char* buffer) const
{
	const char* pBuf = buffer;
	int count = 0;

	while (*pBuf != '\0')
	{
		if (*pBuf == '\n')
			++count;

		++pBuf;
	}

	return count - 1;
}

int CSVFile::countCol(const char* buffer) const
{
	const char* pBuf = buffer;
	int count = 0;

	while (*pBuf != '\n')
	{
		if (*pBuf == ',')
			++count;

		++pBuf;
	}

	return count + 1;
}
