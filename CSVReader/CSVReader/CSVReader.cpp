#pragma once
#include "CSVReader.h"
#include <stdio.h>
#include <memory>

CSVFile::CSVFile(const char* fileName)
	: mFileName(fileName)
	, mLineLength(0)
	, mMemory(nullptr)
{
	
}

CSVFile::~CSVFile()
{
	if (mMemory == nullptr)
	{
		return;
	}

	for (int i = 0; i < mLineLength; ++i)
	{
		delete[] mMemory[i];
	}
}

void CSVFile::readFile()
{
	FILE* fin;

	fopen_s(&fin, mFileName, "r");
	fseek(fin, 0, SEEK_END);

	int size = ftell(fin);
	char* buffer = new char[size];
	memset(buffer, 0, size);
	fseek(fin, 0, SEEK_SET);
	fread_s(buffer, size, size, 1, fin);

	int lineSize = countLine(buffer);



	delete[] buffer;
	fclose(fin);
}

int CSVFile::countLine(const char* buffer) const
{
	const char* pBuf = buffer;
	int count = 0;

	while (*pBuf != '\0')
	{
		if (*pBuf == '\n')
			++count;

		++pBuf;
	}

	return count;
}
