#pragma once
#include "TxtReader.h"
#include "operatorNewOverload.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define FILE_NAME_SIZE (64)

TxtReader::TxtReader(const char* fileName)
	: mFileName(fileName)
	, mBuffer(nullptr)
{
	SaveFileToBuffer();
}

TxtReader::~TxtReader()
{
	if (mBuffer != nullptr)
		delete[] mBuffer;
}

void TxtReader::GetNextLine(char** pBegin, char** pEnd, char* buffer)
{
	while (**pEnd != '\n' && **pEnd != '\0')
	{
		++(*pEnd);
	}

	memcpy(buffer, *pBegin, *pEnd - *pBegin);

	++(*pEnd);
}

void TxtReader::ReadNextParagraph(char** pBegin, char** pEnd, int* lineSize, char*** dest)
{
	char buffer[FILE_NAME_SIZE] = { 0, };

	GetNextLine(pBegin, pEnd, buffer);
	int FileListSize = atoi(buffer);

	*dest = (char**)(new char* [FileListSize]);

	for (int i = 0; i < FileListSize; ++i)
	{
		*pBegin = *pEnd;
		GetNextLine(pBegin, pEnd, buffer);
		int size = *pEnd - *pBegin;
		(*dest)[i] = new char[size];

		memcpy((*dest)[i], buffer, size);
		(*dest)[i][size - 1] = '\0';
	}
}

void TxtReader::SaveFileToBuffer()
{
	FILE* fin;
	char buffer[FILE_NAME_SIZE] = { 0, };

	fopen_s(&fin, mFileName, "r");

	fseek(fin, 0, SEEK_END);
	int file_size = ftell(fin);

	mBuffer = new char[file_size];

	memset(mBuffer, 0, file_size);

	fseek(fin, 0, SEEK_SET);
	fread_s(mBuffer, file_size, file_size, 1, fin);
}
